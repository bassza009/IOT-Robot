#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include "Settings.h"

using namespace settings;

// Same wiring as robotcurclerun_raw. Forward = first pin LOW, second HIGH.
const int LEFT_PWM = 5, LEFT_A = 6, LEFT_B = 7;
const int RIGHT_PWM = 10, RIGHT_A = 9, RIGHT_B = 8;
const int SENSOR_PINS[5] = {A5, A4, A3, A2, A1};

static_assert(BASE_SPEED > 0 && BASE_SPEED <= MAX_SPEED && MAX_SPEED <= 255,
              "Use motor speeds in 1..MAX_SPEED, with MAX_SPEED <= 255");
static_assert(TURN_SPEED > 0 && TURN_SPEED <= MAX_SPEED &&
              SEARCH_SPEED > 0 && SEARCH_SPEED <= MAX_SPEED &&
              REVERSE_SPEED > 0 && REVERSE_SPEED <= MAX_SPEED &&
              CROSS_SPEED > 0 && CROSS_SPEED <= MAX_SPEED,
              "Turn/search/reverse/cross speeds must be in 1..MAX_SPEED");
static_assert(RIGHT_TRIM > 0 && RIGHT_TRIM <= 2 && STEERING_KP >= 0,
              "Check RIGHT_TRIM and STEERING_KP");
static_assert(SAMPLE_MS > 0 && SAMPLE_MS < CONTROL_GAP_MS,
              "SAMPLE_MS must be positive and smaller than CONTROL_GAP_MS");

int raw[5] = {}, lineMask = 0, lineGroups = 0;
float lineError = 0; // -2 = far left, 0 = center, +2 = far right.
int leftPwm = 256, rightPwm = 256; // Force initial motor output update.
int lastDirection = 0;           // -1 left, +1 right, 0 unknown.
bool stopped = false, turning = false, crossing = false;
enum class Recovery { None, BrakeBefore, Reverse, BrakeAfter, WaitLine };
Recovery recovery = Recovery::None;
const char* state = "READY";
const char* lastError = "NONE";
uint32_t lastSample = 0, lastLine = 0, turnSince = 0, blackSince = 0;
uint32_t brakeSince = 0, reverseSince = 0;

int limitPwm(int value) {
  if (value > MAX_SPEED) return MAX_SPEED;
  if (value < -MAX_SPEED) return -MAX_SPEED;
  return value;
}

void setWheel(int enable, int a, int b, int value, int previous) {
  if (value == previous) return;
  bool sameDirection = (value > 0 && previous > 0) || (value < 0 && previous < 0);
  if (!sameDirection) {
    analogWrite(enable, 0); // Disable the bridge before changing direction.
    digitalWrite(a, value > 0 ? LOW : HIGH);
    digitalWrite(b, value < 0 ? LOW : HIGH);
  }
  // Zero command = both direction pins HIGH, full enable: dynamic braking.
  analogWrite(enable, value == 0 ? 255 : (value < 0 ? -value : value));
}

bool motorPowerAllowed() {
  // Explicit run override; keep the actual supply/driver ratings in Settings.h.
  if (ALLOW_UNVERIFIED_MOTOR_POWER) return true;
  // Check declared ratings only. This cannot measure voltage/current or
  // establish that a replacement driver tolerates starting/braking current.
  return MOTOR_SUPPLY_MV >= 2400 && MOTOR_SUPPLY_MV <= 3000 &&
         DRIVER_DC_LIMIT_MA >= 3000;
}

void drive(int left, int right) {
  if (!motorPowerAllowed()) {
    // Do not enable either bridge, even for dynamic braking, on this setup.
    analogWrite(LEFT_PWM, 0); analogWrite(RIGHT_PWM, 0);
    digitalWrite(LEFT_A, LOW); digitalWrite(LEFT_B, LOW);
    digitalWrite(RIGHT_A, LOW); digitalWrite(RIGHT_B, LOW);
    leftPwm = rightPwm = 0;
    return;
  }
  left = limitPwm(left);
  right = limitPwm(int(right * RIGHT_TRIM));
  setWheel(LEFT_PWM, LEFT_A, LEFT_B, left, leftPwm);
  setWheel(RIGHT_PWM, RIGHT_A, RIGHT_B, right, rightPwm);
  leftPwm = left;
  rightPwm = right;
}

void stopRobot(const char* reason) {
  if (!stopped) state = reason;
  stopped = true; // Only physical RESET starts another run.
  drive(0, 0);
}

void serviceSerial() {
  static int matched = 0;
  const char* command = "STOP";
  // Accept STOP without a newline, across loop calls, and in either case.
  for (int i = 0; i < 16 && Serial.available(); ++i) {
    int ch = toupper(Serial.read());
    matched = ch == command[matched] ? matched + 1 : (ch == 'S' ? 1 : 0);
    if (matched == 4) { stopRobot("SERIAL_STOP"); matched = 0; }
  }
}

int readSensor(int pin) {
  analogRead(pin); // Discard first reading after switching ADC channels.
  int a = analogRead(pin), b = analogRead(pin), c = analogRead(pin);
  if (a > b) { int t = a; a = b; b = t; }
  if (b > c) { int t = b; b = c; c = t; }
  return a > b ? a : b; // Median of three rejects a single noisy reading.
}

void readLine() {
  lineMask = lineGroups = 0;
  float weight = 0, weightedPosition = 0;
  bool previousBlack = false;
  for (int i = 0; i < 5; ++i) {
    raw[i] = readSensor(SENSOR_PINS[i]);
    int span = BLACK_RAW[i] - FLOOR_RAW[i];
    float darkness = span == 0 ? 0 : 1000.0f * (raw[i] - FLOOR_RAW[i]) / span;
    if (darkness < 0) darkness = 0;
    if (darkness > 1000) darkness = 1000;
    bool black = darkness >= LINE_THRESHOLD;
    if (black) {
      lineMask |= 1 << i;
      if (!previousBlack) ++lineGroups;
      weight += darkness;
      weightedPosition += (i - 2) * darkness;
    }
    previousBlack = black;
  }
  lineError = weight > 0 ? weightedPosition / weight : 0;
}

void pivot(int direction, int speed) {
  drive(direction * speed, -direction * speed);
}

void startRecovery(uint32_t now, const char* reason) {
  lastError = reason;
  recovery = Recovery::BrakeBefore;
  brakeSince = now;
  turning = crossing = false;
  state = "ERROR_BRAKE";
  drive(0, 0);
}

// Returns true while recovery owns the motors; STOP is checked by followLine().
bool recoverLine(uint32_t now) {
  if (recovery == Recovery::None) return false;
  if (recovery == Recovery::BrakeBefore) {
    if (now - brakeSince < BRAKE_MS) return true;
    recovery = Recovery::Reverse;
    reverseSince = now;
    state = "REVERSE";
    drive(-REVERSE_SPEED, -REVERSE_SPEED);
    return true;
  }

  // Resume only on a normal tracking pattern, not the same failed outer turn,
  // separated lines, or an all-black strip. No encoder or delay() is needed.
  bool found = lineGroups == 1 && lineMask != 31 && fabsf(lineError) < 1.5f &&
               lineMask != 7 && lineMask != 15 && lineMask != 28 && lineMask != 30;
  if (recovery == Recovery::WaitLine && !found) return true;
  if ((recovery == Recovery::Reverse || recovery == Recovery::WaitLine) && found) {
    recovery = Recovery::BrakeAfter;
    brakeSince = now;
    state = "LINE_BRAKE";
    drive(0, 0);
  }
  if (recovery == Recovery::BrakeAfter) {
    if (now - brakeSince < BRAKE_MS) return true;
    if (found) { recovery = Recovery::None; return false; }
    recovery = Recovery::Reverse; // Brief sighting vanished: keep the old deadline.
  }
  if (now - reverseSince >= REVERSE_MS) {
    // End this reverse attempt, but keep reading sensors so RESET is not needed.
    recovery = Recovery::WaitLine;
    state = "WAIT_LINE";
    drive(0, 0);
  } else if (recovery == Recovery::Reverse) {
    state = "REVERSE";
    drive(-REVERSE_SPEED, -REVERSE_SPEED);
  }
  return true;
}

// Main behavior: change these small branches to change how the robot follows.
void followLine(uint32_t now) {
  if (stopped) return;
  if (recoverLine(now)) return;

  if (lineMask == 31) {
    if (!crossing) { crossing = true; blackSince = now; }
    turning = false;
    if (now - blackSince >= ALL_BLACK_MS) startRecovery(now, "ALL_BLACK");
    else { state = "CROSS"; drive(CROSS_SPEED, CROSS_SPEED); }
    return;
  }
  crossing = false;

  // Separate black groups may be neighboring lines. Do not average across them.
  if (lineGroups > 1) { startRecovery(now, "AMBIGUOUS_LINE"); return; }
  if (lineMask == 0) {
    if (lastDirection == 0 || now - lastLine >= SEARCH_MS ||
        (turning && now - turnSince >= TURN_TIMEOUT_MS)) {
      startRecovery(now, "LINE_LOST");
    } else {
      state = "SEARCH";
      pivot(lastDirection, SEARCH_SPEED);
    }
    return;
  }
  lastLine = now;
  if (fabsf(lineError) > 0.1f) lastDirection = lineError < 0 ? -1 : 1;

  int turn = 0;
  if (lineMask == 7 || lineMask == 15 || lineError <= -1.5f) turn = -1;
  if (lineMask == 28 || lineMask == 30 || lineError >= 1.5f) turn = 1;
  if (turn != 0) {
    if (!turning) { turning = true; turnSince = now; }
    lastDirection = turn;
    if (now - turnSince >= TURN_TIMEOUT_MS) startRecovery(now, "TURN_TIMEOUT");
    else { state = "TURN"; pivot(turn, TURN_SPEED); }
    return;
  }
  turning = false;

  state = "FOLLOW";
  int correction = int(STEERING_KP * lineError);
  // Left line: negative correction slows the left wheel, speeds up the right.
  int left = BASE_SPEED + correction, right = BASE_SPEED - correction;
  drive(left < 0 ? 0 : left, right < 0 ? 0 : right);
}

void reportStatus(uint32_t now) {
  static char report[160];
  static size_t length = 0, sent = 0;
  static uint32_t lastReport = 0;
  if (!Serial) { length = sent = 0; return; }
  if (sent == length && now - lastReport >= REPORT_MS) {
    lastReport = now;
    int n = snprintf(report, sizeof(report),
      "MS=%lu STATE=%s LAST_ERROR=%s RAW=%d,%d,%d,%d,%d MASK=%02X ERR=%d PWM=%d,%d\n",
      (unsigned long)now, state, lastError, raw[0], raw[1], raw[2], raw[3], raw[4],
      unsigned(lineMask), int(lineError * 1000), leftPwm, rightPwm);
    length = n > 0 ? (size_t(n) < sizeof(report) ? size_t(n) : sizeof(report) - 1) : 0;
    sent = 0;
  }
  int room = Serial.availableForWrite();
  size_t count = length - sent;
  if (room > 0 && count > 0) {
    if (count > size_t(room)) count = size_t(room);
    sent += Serial.write((const uint8_t*)report + sent, count);
  }
}

void updateStatusLED(uint32_t now) {
  bool on = true;
  if (stopped)
    on = now % 1500 < 100 || (now % 1500 >= 250 && now % 1500 < 350);
  else if (recovery == Recovery::WaitLine) on = now % 1000 < 150;
  else if (recovery == Recovery::Reverse) on = (now / 100) % 2 == 0;
  else if (recovery != Recovery::None) on = false; // Braking before direction change.
  digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
}

void setup() {
  analogWriteResolution(8);
  pinMode(LEFT_PWM, OUTPUT); pinMode(RIGHT_PWM, OUTPUT);
  analogWrite(LEFT_PWM, 0); analogWrite(RIGHT_PWM, 0);
  pinMode(LEFT_A, OUTPUT); pinMode(LEFT_B, OUTPUT);
  pinMode(RIGHT_A, OUTPUT); pinMode(RIGHT_B, OUTPUT);
  drive(0, 0);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  analogReadResolution(12);
  for (int i = 0; i < 5; ++i) {
    pinMode(SENSOR_PINS[i], INPUT);
    if (FLOOR_RAW[i] < 0 || FLOOR_RAW[i] > 4095 ||
        BLACK_RAW[i] < 0 || BLACK_RAW[i] > 4095 || FLOOR_RAW[i] == BLACK_RAW[i])
      stopRobot("INVALID_SENSOR_CONFIG");
  }
  if (LINE_THRESHOLD < 1 || LINE_THRESHOLD > 1000) stopRobot("INVALID_THRESHOLD");
  if (!motorPowerAllowed()) stopRobot("MOTOR_POWER_CONFIG");
  lastSample = lastLine = millis();
}

void loop() {
  serviceSerial(); // STOP works even between sensor samples.
  uint32_t now = millis();
  if (now - lastSample >= SAMPLE_MS) {
    readLine();
    now = millis();
    if ((leftPwm != 0 || rightPwm != 0) && now - lastSample > CONTROL_GAP_MS)
      stopRobot("CONTROL_OVERRUN");
    lastSample = now;
    followLine(now);
    updateStatusLED(now);
  }
  reportStatus(millis());
}
