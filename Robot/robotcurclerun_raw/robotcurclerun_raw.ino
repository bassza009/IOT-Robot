#include <ArduinoBLE.h>
#include "LineFollower.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

// Wiring preserved from the user's latest sketch (not the older review).
// Left wheel: D5 enable, D6/D7 direction. Right: D10 enable, D9/D8 direction.
const int ENA = 5;
const int IN1 = 6;
const int IN2 = 7;
const int ENB = 10;
const int IN3 = 9;
const int IN4 = 8;
const int ENCODER_L = 3;
const int ENCODER_R = 11;
const int SENSOR_PINS[5] = {A5, A4, A3, A2, A1}; // physical left -> right
const bool USE_BLE = true;
const uint32_t SAMPLE_MS = 3;
const uint32_t REPORT_INTERVAL_MS = 100; // ส่งข้อมูลออก Serial อย่างต่อเนื่องทุก 100 ms (10 ครั้ง/วิ)

// Set manual wood/black readings, speeds and recovery bounds in LineFollower.h.
linebot::Config config;
linebot::Controller robot(config);
volatile uint32_t pulseCountL = 0;
volatile uint32_t pulseCountR = 0;
int rawSensors[5] = {};
int appliedLeft = 0;
int appliedRight = 0;
bool appliedBrake = false;

BLEService uartService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLECharacteristic txChar("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify, 20);
BLECharacteristic rxChar("6E400002-B5A3-F393-E0A9-E50E24DCCA9E",
                         BLEWrite | BLEWriteWithoutResponse, 64);
bool bleReady = false;
uint32_t lastSampleMs = 0;
uint32_t lastBlePollMs = 0;
uint32_t lastReportMs = 0;
uint32_t lastTxMs = 0;
linebot::State lastReportedState = linebot::State::Countdown;

// Latest status snapshot. Each transport finishes its own in-flight frame;
// rapid state changes only replace the snapshot waiting to be sent next.
// BLE notifications are sent only while stationary; RX stays serviced in motion.
char report[512] = {};
size_t reportLength = 0;

struct StatusOutput {
  char data[512] = {};
  size_t length = 0;
  size_t offset = 0;
  bool pending = false;

  void prepare(const char* latest, size_t size) {
    if (offset < length || !pending) return;
    memcpy(data, latest, size);
    length = size;
    offset = 0;
    pending = false;
  }

  void reset() {
    length = offset = 0;
    pending = true;
  }
};

StatusOutput serialStatus;
StatusOutput bleStatus;

void countLeft() { ++pulseCountL; }
void countRight() { ++pulseCountR; }

void setWheel(int enable, int inA, int inB, int pwm, bool brake, int previousPwm) {
  if ((pwm > 0 && previousPwm > 0) || (pwm < 0 && previousPwm < 0)) {
    analogWrite(enable, pwm > 0 ? pwm : -pwm);
    return;
  }
  // Disable the bridge before changing a direction input.
  analogWrite(enable, 0);
  if (pwm == 0) {
    digitalWrite(inA, brake ? HIGH : LOW);
    digitalWrite(inB, brake ? HIGH : LOW);
    if (brake) analogWrite(enable, 65535);
  } else {
    digitalWrite(inA, pwm > 0 ? LOW : HIGH);
    digitalWrite(inB, pwm > 0 ? HIGH : LOW);
    analogWrite(enable, pwm > 0 ? pwm : -pwm);
  }
}

void applyDrive() {
  const linebot::Drive drive = robot.drive();
  if (drive.left != appliedLeft || drive.brake != appliedBrake)
    setWheel(ENA, IN1, IN2, drive.left, drive.brake, appliedLeft);
  if (drive.right != appliedRight || drive.brake != appliedBrake)
    setWheel(ENB, IN3, IN4, drive.right, drive.brake, appliedRight);
  appliedLeft = drive.left;
  appliedRight = drive.right;
  appliedBrake = drive.brake;
}

void stopFromBLE(BLEDevice central, BLECharacteristic characteristic) {
  (void)central;
  // Event handler observes EACH write, including STOP followed rapidly by START.
  // START intentionally has no restart behavior: a stop is latched until reset.
  char command[65];
  int length = characteristic.valueLength();
  if (length > 64) length = 64;
  const uint8_t* value = characteristic.value();
  int used = 0;
  for (int i = 0; i < length; ++i) {
    if (!isspace(value[i])) command[used++] = char(toupper(value[i]));
  }
  command[used] = '\0';
  if (strcmp(command, "STOP") == 0) {
    robot.stop(linebot::Reason::BleStop);
    applyDrive();
  }
}

void serviceSerialCommands() {
  static char command[16];
  static int used = 0;
  // Bounded input processing so a noisy serial sender cannot starve motor control.
  for (int i = 0; i < 16 && Serial.available(); ++i) {
    int ch = Serial.read();
    if (ch == '\n' || ch == '\r') {
      used = 0;
    } else if (ch >= 0 && used < 15) {
      if (used == 0 && isspace(ch)) continue; // ignore leading whitespace
      command[used++] = char(toupper(ch));
      command[used] = '\0';
      if (strcmp(command, "STOP") == 0) {
        robot.stop(linebot::Reason::SerialStop);
        applyDrive();
        used = 0;
      }
    }
  }
}

int readSensor(int pin) {
  // Discard the first conversion after changing ADC channel, then use median-3.
  analogRead(pin);
  int a = analogRead(pin);
  int b = analogRead(pin);
  int c = analogRead(pin);
  if (a > b) { int t = a; a = b; b = t; }
  if (b > c) { int t = b; b = c; c = t; }
  if (a > b) b = a;
  return b;
}

void queueStatus(uint32_t encL, uint32_t encR) {
  linebot::Observation line = robot.observation();
  int n = snprintf(report, sizeof(report),
    "\nMS=%lu STATE=%s REASON=%s SENSOR=MANUAL MASK=%02X ERR_MILLI=%d\n"
    "SENSORS: A5:%-4d  A4:%-4d  A3:%-4d  A2:%-4d  A1:%-4d\n"
    "PWM_L=%d PWM_R=%d ENC_L=%lu ENC_R=%lu\n"
    "RAW=%d,%d,%d,%d,%d\n"
    "WOOD=%d,%d,%d,%d,%d BLACK=%d,%d,%d,%d,%d\n",
    (unsigned long)millis(), linebot::stateName(robot.state()), linebot::reasonName(robot.reason()),
    unsigned(line.mask), int(line.position * 1000),
    rawSensors[0], rawSensors[1], rawSensors[2], rawSensors[3], rawSensors[4],
    appliedLeft, appliedRight, (unsigned long)encL, (unsigned long)encR,
    rawSensors[0], rawSensors[1], rawSensors[2], rawSensors[3], rawSensors[4],
    robot.wood(0), robot.wood(1), robot.wood(2), robot.wood(3), robot.wood(4),
    robot.black(0), robot.black(1), robot.black(2), robot.black(3), robot.black(4));
  reportLength = n < 0 ? 0 : (size_t(n) < sizeof(report) ? size_t(n) : sizeof(report) - 1);
  serialStatus.pending = bleStatus.pending = true;
}

void flushStatus(uint32_t now) {
  if (Serial) {
    serialStatus.prepare(report, reportLength);
    size_t n = serialStatus.length - serialStatus.offset;
    int available = Serial.availableForWrite();
    if (available > 0 && n > 0) {
      if (n > size_t(available)) n = size_t(available);
      serialStatus.offset += Serial.write(
        (const uint8_t*)serialStatus.data + serialStatus.offset, n);
    }
  } else {
    serialStatus.reset();
  }
  if (!bleReady || !txChar.subscribed()) {
    bleStatus.reset();
    return;
  }
  if (linebot::Controller::isMoving(robot.state())) return;
  bleStatus.prepare(report, reportLength);
  if (bleStatus.offset < bleStatus.length && now - lastTxMs >= 30) {
    size_t n = bleStatus.length - bleStatus.offset;
    if (n > 20) n = 20;
    lastTxMs = now;
    if (txChar.writeValue((const uint8_t*)bleStatus.data + bleStatus.offset, n))
      bleStatus.offset += n;
  }
}

void updateStatusLED(uint32_t now) {
  const linebot::State state = robot.state();
  bool on = false;
  if (state == linebot::State::Countdown)
    on = (now / 500) % 2 == 0;
  else if (state == linebot::State::Stopped || state == linebot::State::Fault)
    on = now % 1500 < 100 || (now % 1500 >= 250 && now % 1500 < 350);
  else on = linebot::Controller::isMoving(state) ||
            state == linebot::State::RecoveryBrake || state == linebot::State::AlignBrake;
  digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
}

void setup() {
  // Motor outputs become inactive before potentially slow peripheral setup.
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  analogWriteResolution(16);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  analogReadResolution(12);
  for (int i = 0; i < 5; ++i) pinMode(SENSOR_PINS[i], INPUT);
  pinMode(ENCODER_L, INPUT_PULLUP);
  pinMode(ENCODER_R, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_L), countLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_R), countRight, RISING);

  // Initialize before advertising: an early BLE STOP must never be cleared
  // by a later initialization step during setup().
  robot.begin(millis());
  if (USE_BLE) {
    bleReady = BLE.begin();
    if (bleReady) {
      BLE.setLocalName("IOT-ROBOT");
      BLE.setDeviceName("IOT-ROBOT");
      BLE.setAdvertisedService(uartService);
      uartService.addCharacteristic(txChar);
      uartService.addCharacteristic(rxChar);
      BLE.addService(uartService);
      rxChar.setEventHandler(BLEWritten, stopFromBLE);
      BLE.advertise();
    }
  }
  uint32_t now = millis();
  if (USE_BLE && !bleReady) robot.fault(linebot::Reason::BleUnavailable);
  robot.startCountdown(now);
  applyDrive();
  lastSampleMs = lastBlePollMs = lastReportMs = now;
  lastReportedState = robot.state();
  serialStatus.reset();
  bleStatus.reset();
  queueStatus(0, 0);
}

void loop() {
  serviceSerialCommands();
  uint32_t now = millis();
  if (bleReady && now - lastBlePollMs >= SAMPLE_MS) {
    lastBlePollMs = now;
    BLE.poll();
  }
  now = millis();
  if (now - lastSampleMs >= SAMPLE_MS) {
    lastSampleMs = now;
    for (int i = 0; i < 5; ++i) rawSensors[i] = readSensor(SENSOR_PINS[i]);
    noInterrupts();
    uint32_t encL = pulseCountL;
    uint32_t encR = pulseCountR;
    interrupts();
    now = millis();
    robot.tick(now, rawSensors, encL, encR);
    applyDrive();
    bool changed = robot.state() != lastReportedState;
    if (changed || now - lastReportMs >= REPORT_INTERVAL_MS) {
      queueStatus(encL, encR);
      lastReportedState = robot.state();
      lastReportMs = now;
    }
    updateStatusLED(now);
  }
  flushStatus(millis());
}
