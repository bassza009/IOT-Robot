// Includes the production sketch, replacing only the Arduino hardware boundary.
#include "../robotcurclerun_raw.ino"
#include <stdio.h>
#include <stdlib.h>

static int checks = 0;
#define CHECK(condition) do { ++checks; if (!(condition)) { \
  fprintf(stderr, "FAIL sketch line %d: %s\n", __LINE__, #condition); exit(1); } } while (0)

void step(int mask) {
  for (int i = 0; i < 5; ++i)
    fakeAnalog[SENSOR_PINS[i]] = mask & (1 << i) ? config.blackRaw[i] : config.woodRaw[i];
  fakeTime += 5;
  loop();
}

void startRun() {
  fakeTime = 0;
  appliedLeft = appliedRight = 0;
  appliedBrake = false;
  setup();
  CHECK(fakeWriteResolution == 16 && fakeReadResolution == 12);
  CHECK(fakePwm[ENA] == 0 && fakePwm[ENB] == 0);
  for (int i = 0; i < 5; ++i) CHECK(fakeMode[SENSOR_PINS[i]] == INPUT);
  CHECK(robot.state() == linebot::State::Countdown);
  CHECK(strstr(report, "SENSOR=MANUAL") != nullptr);
  CHECK(config.countdownMs == 0);
  CHECK(robot.state() == linebot::State::Countdown);
  CHECK(appliedLeft == 0 && appliedRight == 0);
  step(4);
  CHECK(robot.state() == linebot::State::Follow);
  CHECK(appliedLeft > 0 && appliedRight > 0);
  step(4);
}

void checkBraked() {
  CHECK(fakeDigital[IN1] == HIGH && fakeDigital[IN2] == HIGH);
  CHECK(fakeDigital[IN3] == HIGH && fakeDigital[IN4] == HIGH);
  CHECK(fakePwm[ENA] == 65535 && fakePwm[ENB] == 65535);
  CHECK(appliedLeft == 0 && appliedRight == 0);
}

void statusFrameTests() {
  startRun();
  Serial.output.clear();
  for (int i = 0; i < 120; ++i) step(i % 2 ? 1 : 4);
  CHECK(Serial.output.find("RAW=") != std::string::npos);
  CHECK(Serial.output.find(" BLACK=") != std::string::npos);

  // Replay the reported RECOVERY_BRAKE -> ALIGN_BRAKE cycle. BLE can only
  // transmit during the short braking windows, so a frame spans several.
  startRun();
  txChar.output.clear();
  for (int cycle = 0; cycle < 10; ++cycle) {
    for (int i = 0; i < 22; ++i) step(0);
    step(4);
    for (int i = 0; i < 16; ++i) step(4);
  }
  CHECK(txChar.output.find("RAW=") != std::string::npos);
  CHECK(txChar.output.find(" BLACK=") != std::string::npos);
}

int main() {
  statusFrameTests();
  // Start on the first sensor sample after BLE setup, with no extra delay.
  BLE.beginDelayMs = 7000;
  startRun();
  CHECK(ENA == 5 && IN1 == 6 && IN2 == 7);
  CHECK(ENB == 10 && IN3 == 9 && IN4 == 8);
  CHECK(SENSOR_PINS[0] == A5 && SENSOR_PINS[4] == A1);
  CHECK(SENSOR_PINS[1] == A4 && SENSOR_PINS[3] == A2);
  CHECK(fakeDigital[IN1] == LOW && fakeDigital[IN2] == HIGH);
  CHECK(fakeDigital[IN3] == LOW && fakeDigital[IN4] == HIGH);
  int offBefore = fakeZeroPwmWrites;
  step(2); // PWM correction must not repeatedly disable an already-forward motor.
  CHECK(fakeZeroPwmWrites == offBefore);
  step(1);
  CHECK(robot.state() == linebot::State::Turn);
  CHECK(fakeDigital[IN1] == HIGH && fakeDigital[IN2] == LOW);
  CHECK(fakeDigital[IN3] == LOW && fakeDigital[IN4] == HIGH);
  step(16); // A5 -> A1: left forward, right reverse.
  CHECK(fakeDigital[IN1] == LOW && fakeDigital[IN2] == HIGH);
  CHECK(fakeDigital[IN3] == HIGH && fakeDigital[IN4] == LOW);
  step(1); // A1 -> A5: left reverse, right forward, in this same loop.
  CHECK(fakeDigital[IN1] == HIGH && fakeDigital[IN2] == LOW);
  CHECK(fakeDigital[IN3] == LOW && fakeDigital[IN4] == HIGH);
  CHECK(fakePwm[ENA] == config.turnPwm);
  CHECK(fakePwm[ENB] == int(config.turnPwm * config.rightTrim));
  step(2); // A4 resumes a forward left curve.
  CHECK(fakeDigital[IN1] == LOW && fakeDigital[IN2] == HIGH);
  CHECK(fakeDigital[IN3] == LOW && fakeDigital[IN4] == HIGH);
  CHECK(fakePwm[ENA] < fakePwm[ENB] / config.rightTrim);
  step(16);
  step(8); // A2 is the mirrored forward right curve.
  CHECK(fakeDigital[IN1] == LOW && fakeDigital[IN2] == HIGH);
  CHECK(fakeDigital[IN3] == LOW && fakeDigital[IN4] == HIGH);
  CHECK(fakePwm[ENA] > fakePwm[ENB] / config.rightTrim);
  step(1);
  int notifications = txChar.notifications;
  flushStatus(fakeTime + 50);
  CHECK(txChar.notifications == notifications); // No blocking BLE TX while turning.
  rxChar.simulateWrite(" stop\r\n");
  CHECK(robot.state() == linebot::State::Stopped);
  checkBraked(); // Callback brakes immediately, before the next loop/tick.
  rxChar.simulateWrite("START");
  for (int i = 0; i < 100; ++i) step(4);
  CHECK(robot.state() == linebot::State::Stopped);
  CHECK(robot.reason() == linebot::Reason::BleStop);
  checkBraked();

  startRun();
  for (int i = 0; i < 30; ++i) step(0);
  CHECK(robot.state() == linebot::State::Reverse);
  CHECK(fakeDigital[IN1] == HIGH && fakeDigital[IN2] == LOW);
  CHECK(fakeDigital[IN3] == HIGH && fakeDigital[IN4] == LOW);
  Serial.input = "STOP\n";
  loop();
  CHECK(robot.reason() == linebot::Reason::SerialStop);
  checkBraked();

  BLE.beginOK = false;
  // A physical reset also restores the sketch's cached output globals.
  appliedLeft = appliedRight = 0;
  appliedBrake = false;
  setup();
  CHECK(robot.state() == linebot::State::Fault);
  CHECK(robot.reason() == linebot::Reason::BleUnavailable);
  checkBraked();
  CHECK(BLE.polls > 0);

  BLE.beginOK = true;
  BLE.onAdvertise = []() { rxChar.simulateWrite("STOP"); };
  appliedLeft = appliedRight = 0;
  appliedBrake = false;
  setup();
  CHECK(robot.state() == linebot::State::Stopped);
  CHECK(robot.reason() == linebot::Reason::BleStop);
  checkBraked();
  printf("PASS sketch: %d checks (actual setup/loop, wiring, BLE callback, serial STOP, PWM)\n", checks);
}
