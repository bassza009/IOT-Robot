#include "../LineFollower.h"
#include <stdio.h>
#include <stdlib.h>

using namespace linebot;
static int checks = 0;
#define CHECK(condition) do { ++checks; if (!(condition)) { \
  fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); exit(1); } } while (0)

struct Fixture {
  Config cfg;
  Controller robot;
  uint32_t now, encL = 0, encR = 0;
  int wood[5] = {3000, 3000, 3000, 3000, 3000};
  int black[5] = {300, 300, 300, 300, 300};
  explicit Fixture(uint32_t start = 0, const Config& settings = Config())
      : cfg(settings), robot(cfg), now(start) {
    for (int i = 0; i < 5; ++i) { wood[i] = cfg.woodRaw[i]; black[i] = cfg.blackRaw[i]; }
    robot.begin(now);
  }
  void step(int mask = 0) {
    int raw[5];
    for (int i = 0; i < 5; ++i) raw[i] = mask & (1 << i) ? black[i] : wood[i];
    now += 5;
    robot.tick(now, raw, encL, encR);
  }
  void run(uint32_t ms, int mask = 0) { for (uint32_t i = 0; i < ms; i += 5) step(mask); }
  void follow() {
    CHECK(robot.state() == State::Countdown);
    if (cfg.countdownMs == 0) step(4);
    else run(cfg.countdownMs, 4);
    CHECK(robot.state() == State::Follow);
    step(4);
  }
  void reverse() {
    follow();
    run(cfg.lossConfirmMs + 5, 0);
    CHECK(robot.state() == State::RecoveryBrake);
    run(cfg.brakeMs + 5, 0);
    CHECK(robot.state() == State::Reverse);
    CHECK(robot.drive().left < 0 && robot.drive().right < 0);
  }
};

void continuousTrackingTests() {
  // Every clear sensor position starts on the first sample with no delay.
  const int startMasks[] = {1, 2, 4, 8, 16, 6, 12};
  for (int mask : startMasks) {
    Fixture f;
    CHECK(f.cfg.countdownMs == 0);
    CHECK(f.robot.drive().left == 0 && f.robot.drive().right == 0);
    f.step(mask);
    CHECK(f.robot.drive().left != 0 || f.robot.drive().right != 0);
    CHECK(f.robot.state() == State::Follow || f.robot.state() == State::Turn);
  }

  for (int side = 0; side < 2; ++side) {
    Fixture f;
    f.follow();
    f.step(side == 0 ? 1 : 16);
    CHECK(f.robot.state() == State::Turn);
    f.step(side == 0 ? 6 : 12); // Pick up the line with a center + inner pair.
    CHECK(f.robot.state() == State::Follow);
    CHECK(f.robot.drive().left > 0 && f.robot.drive().right > 0);
    f.step(4);
    CHECK(f.robot.drive().left > 0 && f.robot.drive().right > 0);
  }

  Fixture flicker;
  flicker.follow();
  const Drive before = flicker.robot.drive();
  flicker.step(0);
  flicker.step(0);
  CHECK(flicker.robot.drive().left == before.left);
  CHECK(flicker.robot.drive().right == before.right);
  flicker.step(6);
  CHECK(flicker.robot.state() == State::Follow);
  CHECK(flicker.robot.drive().left > 0 && flicker.robot.drive().right > 0);

  Fixture absent;
  absent.run(absent.cfg.countdownMs + absent.cfg.brakeMs + 5, 0);
  CHECK(absent.robot.state() == State::Reverse);
  CHECK(absent.robot.drive().left < 0 && absent.robot.drive().right < 0);
  absent.step(12);
  CHECK(absent.robot.state() == State::AlignBrake);
  absent.run(absent.cfg.brakeMs, 12);
  CHECK(absent.robot.state() == State::Follow);
  CHECK(absent.robot.drive().left > 0 && absent.robot.drive().right > 0);
}

void directionChangeTests() {
  // A new visible line on the opposite side must replace the old turn command.
  // In particular, touching A5 after A1 must immediately command a LEFT pivot.
  const int leftMasks[] = {1, 2, 3, 6}; // A5, A4, A5+A4, A4+A3
  const int rightMasks[] = {16, 8, 24, 12};
  for (int i = 0; i < 4; ++i) {
    Fixture left, right;
    left.step(16);
    right.step(1);
    left.step(leftMasks[i]);
    right.step(rightMasks[i]);
    CHECK(left.robot.observation().position < 0);
    CHECK(right.robot.observation().position > 0);
    CHECK(left.robot.drive().left < left.robot.drive().right / left.cfg.rightTrim);
    CHECK(right.robot.drive().left > right.robot.drive().right / right.cfg.rightTrim);
    if (i == 0) {
      CHECK(left.robot.drive().left < 0 && left.robot.drive().right > 0);
      CHECK(right.robot.drive().left > 0 && right.robot.drive().right < 0);
    }
    // Mirror the commands after accounting for the user's right-wheel trim.
    CHECK(abs(left.robot.drive().left -
              int(right.robot.drive().right / right.cfg.rightTrim)) <= 2);
    CHECK(abs(right.robot.drive().left -
              int(left.robot.drive().right / left.cfg.rightTrim)) <= 2);
  }

  Fixture lost;
  lost.step(16);
  lost.step(1);
  lost.run(lost.cfg.turnTimeoutMs, 0);
  CHECK(lost.robot.state() == State::RecoveryBrake);
  lost.run(lost.cfg.brakeMs, 0);
  CHECK(lost.robot.state() == State::Reverse);
  lost.step(4);
  lost.run(lost.cfg.brakeMs, 0); // The brief center sighting disappears again.
  CHECK(lost.robot.state() == State::Align);
  CHECK(lost.robot.drive().left < 0 && lost.robot.drive().right > 0);
}

void manualSensorTests() {
  // Fix this test's reference values so hardware tuning does not break it.
  Config analogConfig;
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    analogConfig.woodRaw[i] = 3000;
    analogConfig.blackRaw[i] = 300;
  }
  Fixture analog(0, analogConfig);
  analog.follow();
  CHECK(analog.robot.observation().centered);
  CHECK(analog.robot.wood(0) == 3000 && analog.robot.black(0) == 300);
  CHECK(analog.robot.strength(2) == 1000 && analog.robot.strength(0) == 0);

  Config mixedConfig;
  for (int i = 0; i < 5; ++i) {
    mixedConfig.woodRaw[i] = i % 2 ? 0 : 4095;
    mixedConfig.blackRaw[i] = i % 2 ? 4095 : 0;
  }
  Fixture mixed(0, mixedConfig);
  mixed.follow();
  CHECK(mixed.robot.observation().centered);
  CHECK(mixed.robot.strength(2) == 1000);
  mixed.step(2);
  CHECK(mixed.robot.strength(1) == 1000 && mixed.robot.strength(2) == 0);

  // Every channel uses its own manually entered endpoints, with no learning.
  Config distinct;
  for (int i = 0; i < 5; ++i) {
    distinct.woodRaw[i] = 100 + i * 500;
    distinct.blackRaw[i] = 600 + i * 500;
  }
  Fixture perChannel(0, distinct);
  for (int i = 0; i < 5; ++i) {
    perChannel.step(1 << i);
    CHECK(perChannel.robot.strength(i) == 1000);
    CHECK(perChannel.robot.observation().mask == (1 << i));
    CHECK(perChannel.robot.wood(i) == distinct.woodRaw[i]);
    CHECK(perChannel.robot.black(i) == distinct.blackRaw[i]);
  }

  Config threshold;
  threshold.woodRaw[2] = 0;
  threshold.blackRaw[2] = 1000;
  Fixture boundary(0, threshold);
  boundary.black[2] = 449;
  boundary.step(4);
  CHECK(!boundary.robot.observation().visible);
  boundary.black[2] = 450;
  boundary.step(4);
  CHECK(boundary.robot.observation().centered);
  CHECK(boundary.robot.strength(2) == 450);

  for (int scenario = 0; scenario < 5; ++scenario) {
    Config invalid;
    if (scenario == 0) invalid.blackRaw[4] = invalid.woodRaw[4];
    if (scenario == 1) invalid.blackRaw[0] = -1;
    if (scenario == 2) invalid.woodRaw[1] = 4096;
    if (scenario == 3) invalid.lineThreshold = 0;
    if (scenario == 4) invalid.lineThreshold = 1001;
    Fixture bad(0, invalid);
    CHECK(bad.robot.state() == State::Fault);
    CHECK(bad.robot.reason() == Reason::InvalidSensorConfig);
    bad.robot.startCountdown(bad.now);
    bad.run(6000, 4);
    CHECK(bad.robot.state() == State::Fault);
    CHECK(bad.robot.drive().left == 0 && bad.robot.drive().right == 0);
  }
}

void countdownTests() {
  Config delayed;
  delayed.countdownMs = 5000; // An explicitly configured delay still runs once.
  Fixture f(0, delayed);
  CHECK(f.robot.state() == State::Countdown); // No initial floor/sweep phase.
  f.run(4995, 4);
  CHECK(f.robot.state() == State::Countdown);
  CHECK(f.robot.drive().left == 0 && f.robot.drive().right == 0);
  f.step(4);
  CHECK(f.robot.state() == State::Follow);

  Fixture absent(0, delayed);
  absent.run(5000, 0);
  CHECK(absent.robot.state() == State::RecoveryBrake);
  CHECK(absent.robot.drive().left == 0);
  absent.run(delayed.brakeMs, 6);
  CHECK(absent.robot.state() == State::Follow);
  CHECK(absent.robot.drive().left > 0 && absent.robot.drive().right > 0);
}

void steeringTests() {
  Fixture f;
  f.follow();
  CHECK(f.robot.drive().left == f.cfg.curvePwm);
  f.run(2000, 4);
  CHECK(f.robot.drive().left == f.cfg.cruisePwm);
  CHECK(abs(f.robot.drive().right - int(f.cfg.cruisePwm * f.cfg.rightTrim)) <= 1);
  f.step(2); // Inner left sees black: right wheel must be faster after trim.
  CHECK(f.robot.drive().left < f.robot.drive().right / f.cfg.rightTrim);
  CHECK(f.robot.drive().left < f.cfg.curvePwm);
  f.step(8);
  CHECK(f.robot.drive().left > f.robot.drive().right / f.cfg.rightTrim);

  f.step(1);
  CHECK(f.robot.state() == State::Turn);
  CHECK(f.robot.drive().left < 0 && f.robot.drive().right > 0);
  f.step(4);
  CHECK(f.robot.state() == State::Follow);
  CHECK(f.robot.drive().left > 0 && f.robot.drive().right > 0);
  f.step(4);
  CHECK(f.robot.drive().left == f.cfg.curvePwm);

  Fixture right;
  right.follow();
  right.step(16);
  CHECK(right.robot.state() == State::Turn);
  CHECK(right.robot.drive().left > 0 && right.robot.drive().right < 0);
}

void multiSensorSteeringTests() {
  // Physical left -> right: A5, A4, A3, A2, A1.
  const int masks[] = {0x06, 0x0c, 0x03, 0x18};
  for (int scenario = 0; scenario < 4; ++scenario) {
    Fixture f;
    f.follow();
    const Drive straight = f.robot.drive();
    f.run(200, masks[scenario]);
    CHECK(f.robot.state() == State::Follow);
    CHECK(f.robot.observation().mask == masks[scenario]);
    CHECK(f.robot.drive().left > 0 && f.robot.drive().right > 0);
    if (scenario % 2 == 0) {
      CHECK(f.robot.observation().position < 0);
      CHECK(f.robot.drive().left < straight.left);
      CHECK(f.robot.drive().right > straight.right);
    } else {
      CHECK(f.robot.observation().position > 0);
      CHECK(f.robot.drive().right < straight.right);
      CHECK(f.robot.drive().left > straight.left);
    }
  }

  // A center + inner pair needs less correction than the inner sensor alone.
  for (int side = 0; side < 2; ++side) {
    const int inner = side == 0 ? 0x02 : 0x08;
    Fixture pair, single;
    pair.follow();
    single.follow();
    pair.run(200, inner | 0x04);
    single.run(200, inner);
    if (side == 0) CHECK(pair.robot.drive().left > single.robot.drive().left);
    else CHECK(pair.robot.drive().right > single.robot.drive().right);
  }

  Fixture middleThree;
  middleThree.follow();
  middleThree.run(1000, 0x0e); // A4 + A3 + A2, symmetric about the center.
  CHECK(middleThree.robot.state() == State::Follow);
  CHECK(middleThree.robot.observation().centered);
  CHECK(middleThree.robot.drive().left == middleThree.cfg.cruisePwm);
  CHECK(abs(middleThree.robot.drive().right -
            int(middleThree.cfg.cruisePwm * middleThree.cfg.rightTrim)) <= 1);
}

void cornerTests() {
  // 3-sensor left corner (0x07) and 4-sensor left corner (0x0F)
  const int leftCorners[] = {7, 15};
  for (int c : leftCorners) {
    Fixture f;
    f.follow();
    f.step(c);
    CHECK(f.robot.state() == State::CornerAdvance);
    CHECK(f.robot.observation().corner == -1);
    CHECK(f.robot.drive().left > 0 && f.robot.drive().right > 0);
    f.encL += 5; f.encR += 5;
    f.step(0);
    CHECK(f.robot.state() == State::Turn);
    CHECK(f.robot.drive().left < 0 && f.robot.drive().right > 0);
  }

  // 3-sensor right corner (0x1C) and 4-sensor right corner (0x1E)
  const int rightCorners[] = {28, 30};
  for (int c : rightCorners) {
    Fixture f;
    f.follow();
    f.step(c);
    CHECK(f.robot.state() == State::CornerAdvance);
    CHECK(f.robot.observation().corner == 1);
    CHECK(f.robot.drive().left > 0 && f.robot.drive().right > 0);
    f.encL += 5; f.encR += 5;
    f.step(0);
    CHECK(f.robot.state() == State::Turn);
    CHECK(f.robot.drive().left > 0 && f.robot.drive().right < 0);
  }

  Fixture deadEncoder;
  deadEncoder.follow();
  deadEncoder.step(28);
  deadEncoder.run(deadEncoder.cfg.cornerAdvanceTimeoutMs, 0);
  CHECK(deadEncoder.robot.state() == State::Turn);
  CHECK(deadEncoder.robot.drive().left > 0 && deadEncoder.robot.drive().right < 0);
}

void neighboringLinesTests() {
  Fixture centered;
  centered.follow();
  centered.step(5); // Center line + disconnected far-left black.
  CHECK(centered.robot.observation().centered);
  CHECK(centered.robot.observation().position == 0);
  CHECK(centered.robot.state() == State::Follow);
  centered.step(17); // Two equally plausible outside lines, no center line.
  CHECK(centered.robot.observation().ambiguous);
  CHECK(!centered.robot.observation().centered);
  CHECK(centered.robot.drive().left > 0 && centered.robot.drive().right > 0);
  centered.run(20, 17);
  CHECK(centered.robot.state() == State::RecoveryBrake);
  CHECK(centered.robot.drive().left == 0 && centered.robot.drive().right == 0);

}

void intersectionAndFinishTests() {
  // Scenario 1: Finish line - all 5 sensors black for >= 6cm -> Stop with FinishLine
  {
    Fixture finish;
    finish.follow();
    finish.step(31);
    CHECK(finish.robot.observation().allBlack);
    CHECK(finish.robot.state() == State::IntersectionAdvance);
    CHECK(finish.robot.drive().left > 0 && finish.robot.drive().right > 0);
    // Advance >= 6 cm
    finish.encL += 30; finish.encR += 30;
    finish.step(31);
    CHECK(finish.robot.state() == State::Stopped);
    CHECK(finish.robot.reason() == Reason::FinishLine);
    CHECK(finish.robot.drive().left == 0 && finish.robot.drive().right == 0);
    CHECK(finish.robot.drive().brake == true);
  }

  // Scenario 2: Intersection crossroad - straight line continues (center sensor A3)
  {
    Fixture straightCross;
    straightCross.follow();
    straightCross.step(31);
    CHECK(straightCross.robot.state() == State::IntersectionAdvance);
    CHECK(straightCross.robot.drive().left > 0 && straightCross.robot.drive().right > 0);
    straightCross.step(4); // Only center A3 sees line
    CHECK(straightCross.robot.state() == State::Follow);
    CHECK(straightCross.robot.drive().left > 0 && straightCross.robot.drive().right > 0);
  }

  // Scenario 3: T-junction - left turn (A5 / A4 black, center white)
  {
    Fixture leftTurn;
    leftTurn.follow();
    leftTurn.step(31);
    CHECK(leftTurn.robot.state() == State::IntersectionAdvance);
    leftTurn.step(3); // A5 and A4 black
    CHECK(leftTurn.robot.state() == State::Turn);
    CHECK(leftTurn.robot.drive().left < 0 && leftTurn.robot.drive().right > 0); // Pivot left
  }

  // Scenario 4: T-junction - right turn (A2 / A1 black, center white)
  {
    Fixture rightTurn;
    rightTurn.follow();
    rightTurn.step(31);
    CHECK(rightTurn.robot.state() == State::IntersectionAdvance);
    rightTurn.step(24); // A2 and A1 black
    CHECK(rightTurn.robot.state() == State::Turn);
    CHECK(rightTurn.robot.drive().left > 0 && rightTurn.robot.drive().right < 0); // Pivot right
  }

  // Scenario 5: Dead end / Gap after 5 sensors - recovery
  {
    Fixture deadEnd;
    deadEnd.follow();
    deadEnd.step(31);
    CHECK(deadEnd.robot.state() == State::IntersectionAdvance);
    deadEnd.run(deadEnd.cfg.intersectionAdvanceTimeoutMs + 5, 0);
    CHECK(deadEnd.robot.state() == State::RecoveryBrake);
  }
}

void blindOuterSensorCornerTests() {
  // 90° right turn where A1 has white text:
  // Approaching on A3, then corner appears under A3+A2 (0x0C).
  // Line straight ahead ends, leaving A2 (0x08), then 0.
  // The robot must brake and pivot RIGHT without reversing into an endless loop!
  Fixture rightTrap;
  rightTrap.follow();
  rightTrap.step(12); // A3 + A2
  rightTrap.step(8);  // A2 alone (straight ended)
  rightTrap.run(rightTrap.cfg.lossConfirmMs + 5, 0);
  CHECK(rightTrap.robot.state() == State::RecoveryBrake);
  rightTrap.run(rightTrap.cfg.brakeMs + 5, 0);
  // Instead of entering Reverse, robot immediately pivots RIGHT!
  CHECK(rightTrap.robot.state() == State::Turn);
  CHECK(rightTrap.robot.drive().left > 0 && rightTrap.robot.drive().right < 0);
  // Front swings right onto the new 90° line (A3 catches it)
  rightTrap.step(4);
  CHECK(rightTrap.robot.state() == State::Follow);
  CHECK(rightTrap.robot.drive().left > 0 && rightTrap.robot.drive().right > 0);

  // Mirrored left corner test (if A5 had white text)
  Fixture leftTrap;
  leftTrap.follow();
  leftTrap.step(6); // A4 + A3
  leftTrap.step(2); // A4 alone
  leftTrap.run(leftTrap.cfg.lossConfirmMs + 5, 0);
  CHECK(leftTrap.robot.state() == State::RecoveryBrake);
  leftTrap.run(leftTrap.cfg.brakeMs + 5, 0);
  CHECK(leftTrap.robot.state() == State::Turn);
  CHECK(leftTrap.robot.drive().left < 0 && leftTrap.robot.drive().right > 0); // Pivot left
  leftTrap.step(4);
  CHECK(leftTrap.robot.state() == State::Follow);
  CHECK(leftTrap.robot.drive().left > 0 && leftTrap.robot.drive().right > 0);
}

void recoveryTests() {
  Fixture f;
  f.reverse();
  f.step(1);
  CHECK(f.robot.state() == State::AlignBrake);
  CHECK(f.robot.drive().left == 0 && f.robot.drive().right == 0);
  f.run(f.cfg.brakeMs + 5, 1);
  CHECK(f.robot.state() == State::Align);
  CHECK(f.robot.drive().left < 0 && f.robot.drive().right > 0);
  f.run(65, 4);
  CHECK(f.robot.state() == State::Follow);

  Fixture timeout;
  timeout.reverse();
  timeout.run(timeout.cfg.reverseTimeoutMs, 0);
  CHECK(timeout.robot.state() == State::Fault);
  CHECK(timeout.robot.reason() == Reason::LineNotFound);
  CHECK(timeout.robot.drive().left == 0 && timeout.robot.drive().right == 0);

  Fixture distance;
  distance.reverse();
  distance.encL += 10; distance.encR += 10;
  distance.step(0);
  CHECK(distance.robot.state() == State::Fault);

  Fixture alignFail;
  alignFail.reverse();
  alignFail.step(1);
  alignFail.run(alignFail.cfg.brakeMs + 5, 1);
  alignFail.run(alignFail.cfg.turnTimeoutMs, 1);
  CHECK(alignFail.robot.state() == State::Fault);

  Fixture stuckTurn;
  stuckTurn.follow();
  stuckTurn.step(1);
  stuckTurn.run(stuckTurn.cfg.turnTimeoutMs, 1);
  CHECK(stuckTurn.robot.state() == State::RecoveryBrake);
  stuckTurn.run(stuckTurn.cfg.recoveryTimeoutMs, 1);
  CHECK(stuckTurn.robot.state() == State::Fault);
  CHECK(stuckTurn.robot.reason() == Reason::LineNotFound);
  CHECK(stuckTurn.robot.drive().left == 0 && stuckTurn.robot.drive().right == 0);
}

void stopTests() {
  // Interrupt startup and every drive/recovery state.
  const State states[] = {State::Countdown, State::Follow,
    State::CornerAdvance, State::Turn, State::IntersectionAdvance, State::RecoveryBrake, State::Reverse,
    State::AlignBrake, State::Align};
  for (int scenario = 0; scenario < 9; ++scenario) {
    Fixture f;
    if (scenario >= 1 && scenario <= 5) f.follow();
    if (scenario == 2) f.step(7);
    if (scenario == 3) f.step(1);
    if (scenario == 4) f.step(31);
    if (scenario == 5) f.run(25, 0);
    if (scenario >= 6) f.reverse();
    if (scenario >= 7) f.step(1);
    if (scenario == 8) f.run(85, 1);
    CHECK(f.robot.state() == states[scenario]);
    f.robot.stop(Reason::BleStop);
    CHECK(f.robot.state() == State::Stopped);
    CHECK(f.robot.drive().left == 0 && f.robot.drive().right == 0);
    f.robot.startCountdown(f.now); // Peripheral setup cannot clear an early STOP.
    f.run(10000, 4);
    CHECK(f.robot.state() == State::Stopped);
    CHECK(f.robot.drive().left == 0 && f.robot.drive().right == 0);
  }
}

void timingTests() {
  Fixture rollover(0xfffffff0U);
  rollover.follow();
  CHECK(rollover.robot.state() == State::Follow);
  Fixture gap;
  gap.follow();
  gap.now += 200;
  gap.step(4);
  CHECK(gap.robot.state() == State::Fault);
  CHECK(gap.robot.reason() == Reason::ControlOverrun);

  Fixture bounded;
  bounded.follow();
  for (int mask = 0; mask < 32; ++mask) {
    bounded.step(mask);
    Drive d = bounded.robot.drive();
    CHECK(d.left >= -65535 && d.left <= 65535);
    CHECK(d.right >= -65535 && d.right <= 65535);
  }
}

int main() {
  directionChangeTests();
  continuousTrackingTests();
  manualSensorTests(); countdownTests(); steeringTests(); multiSensorSteeringTests(); cornerTests();
  neighboringLinesTests(); intersectionAndFinishTests(); blindOuterSensorCornerTests(); recoveryTests(); stopTests(); timingTests();
  printf("PASS controller: %d checks (manual sensors, countdown, steering, recovery, STOP, rollover)\n", checks);
}
