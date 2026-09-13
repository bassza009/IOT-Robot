#pragma once

#include <stdint.h>
#include <math.h>

// Hardware-independent controller. Times are milliseconds; PWM is signed 16-bit
// magnitude (positive = forward). Sensor order is physical left -> right.
namespace linebot {

static const int SENSOR_COUNT = 5;

struct Config {
  // MANUAL sensor readings, 12-bit ADC (0..4095), physical left -> right:
  //                          A5    A4    A3    A2    A1
  // Reported RAW: wood ~3700, black 500..670; use 600 as the black reference.
  // Shared starting values; replace individual channels if readings differ.
  int woodRaw[SENSOR_COUNT] = {3000, 3000, 3000, 3000, 3000};
  int blackRaw[SENSOR_COUNT] = {900, 900, 900, 900, 900};
  // Black may be higher OR lower than wood, independently on each channel.
  uint32_t countdownMs = 0;           // Start on the first sample after setup
  int lineThreshold = 450;             // Normalized: wood=0, black=1000
  int cruisePwm = 32800;
  int curvePwm = 32600;
  int turnPwm = 45000;
  int reversePwm = 27000;
  float rightTrim = 46800.0f / 55500.0f; // Preserve the last tested wheel balance
  float steeringKp = 12600.0f;         // PWM per sensor spacing of error
  float steeringKd = 162.0f;          // PWM per spacing/second
  float accelerationPwmPerSec = 10800.0f;
  uint32_t straightStableMs = 300;
  uint32_t lossConfirmMs = 20;
  uint32_t brakeMs = 80;
  uint32_t turnTimeoutMs = 1800;
  uint32_t reverseTimeoutMs = 650;
  uint32_t recoveryTimeoutMs = 3500;
  uint32_t controlGapMs = 150;
  // These encoder distances bound a LOCAL maneuver, never the lap length.
  float cmPerPulse = 3.14159265f * 6.5f / 20.0f;
  float cornerAdvanceCm = 2.5f;        // Sensor row ~6 cm ahead of drive axle
  uint32_t cornerAdvanceTimeoutMs = 250;
  float intersectionAdvanceCm = 6.0f;
  uint32_t intersectionAdvanceTimeoutMs = 250;
  float reverseMaxCm = 10.0f;
};

enum class State {
  Countdown,
  Follow, CornerAdvance, Turn, IntersectionAdvance, RecoveryBrake, Reverse, AlignBrake, Align,
  Stopped, Fault
};

enum class Reason {
  None, BleStop, SerialStop, InvalidSensorConfig, LineNotFound,
  ControlOverrun, BleUnavailable, FinishLine
};

struct Drive {
  int left = 0;
  int right = 0;
  bool brake = false; // true: dynamically brake wheels whose command is zero
};

struct Observation {
  bool visible = false;
  bool ambiguous = false;
  bool centered = false;
  bool allBlack = false;
  int corner = 0; // -1 left, +1 right; a contiguous center-to-outer pattern
  uint8_t mask = 0;
  float position = 0.0f; // -2 = far left, +2 = far right
};

inline float clampFloat(float value, float lo, float hi) {
  return value < lo ? lo : (value > hi ? hi : value);
}

class Controller {
 public:
  explicit Controller(const Config& config = Config()) : cfg(config) {}

  void begin(uint32_t now) {
    state_ = State::Countdown;
    reason_ = Reason::None;
    stateSince = lastTick = now;
    drive_ = Drive();
    obs = Observation();
    straight = missing = recoveryActive = turnSearchDone = false;
    lastPosition = lastError = derivative = 0;
    basePwm = float(cfg.curvePwm);
    turnDirection = 0;
    for (int i = 0; i < SENSOR_COUNT; ++i) {
      strength_[i] = 0;
      if (cfg.woodRaw[i] < 0 || cfg.woodRaw[i] > 4095 ||
          cfg.blackRaw[i] < 0 || cfg.blackRaw[i] > 4095 ||
          cfg.woodRaw[i] == cfg.blackRaw[i]) {
        fault(Reason::InvalidSensorConfig);
      }
    }
    if (cfg.lineThreshold < 1 || cfg.lineThreshold > 1000)
      fault(Reason::InvalidSensorConfig);
  }

  // Optional startup delay begins after setup; zero starts at the first sample.
  // Preserve any STOP received while peripherals were starting.
  void startCountdown(uint32_t now) {
    if (state_ != State::Countdown) return;
    enter(State::Countdown, now);
    lastTick = now;
  }

  // Latched: no serial/BLE command or sensor sample can restart a stopped run.
  // Only begin(), called by setup() after a physical reset, clears the latch.
  void stop(Reason why) {
    if (state_ == State::Stopped || state_ == State::Fault) return;
    state_ = State::Stopped;
    reason_ = why;
    drive_ = Drive();
    drive_.brake = true;
  }

  void fault(Reason why) {
    stop(why);
    state_ = State::Fault;
  }

  State state() const { return state_; }
  Reason reason() const { return reason_; }
  Drive drive() const { return drive_; }
  Observation observation() const { return obs; }
  int wood(int i) const { return cfg.woodRaw[i]; }
  int black(int i) const { return cfg.blackRaw[i]; }
  int strength(int i) const { return strength_[i]; }

  static bool isMoving(State s) {
    return s == State::Follow || s == State::CornerAdvance || s == State::Turn ||
           s == State::IntersectionAdvance || s == State::Reverse || s == State::Align;
  }

  void tick(uint32_t now, const int raw[SENSOR_COUNT], uint32_t encL, uint32_t encR) {
    if (state_ == State::Stopped || state_ == State::Fault) return;
    uint32_t elapsed = now - lastTick;
    lastTick = now;
    if (isMoving(state_) && elapsed > cfg.controlGapMs) {
      fault(Reason::ControlOverrun);
      return;
    }
    float dt = clampFloat(elapsed / 1000.0f, 0.001f, 0.05f);
    const Drive previousDrive = drive_;
    drive_ = Drive();

    obs = observe(raw);
    if (recoveryActive && now - recoverySince >= cfg.recoveryTimeoutMs) {
      fault(Reason::LineNotFound);
      return;
    }

    switch (state_) {
      case State::Countdown:
        drive_.brake = true;
        if (now - stateSince >= cfg.countdownMs) {
          if (obs.visible && !obs.ambiguous) startTracking(now, dt, encL, encR);
          else startRecovery(now);
        }
        break;

      case State::Follow:
        if (obs.allBlack) {
          startEncL = encL;
          startEncR = encR;
          enter(State::IntersectionAdvance, now);
          command(cfg.curvePwm, cfg.curvePwm);
          break;
        }
        if (!obs.visible || obs.ambiguous) {
          // Keep the last steering command through a brief sensor dropout.
          // Persistent loss still enters bounded reverse recovery.
          drive_ = previousDrive;
          if (held(true, now, cfg.lossConfirmMs, missing, missingSince)) {
            startRecovery(now);
          }
          break;
        }
        missing = false;
        trackVisibleLine(now, dt, encL, encR);
        break;

      case State::CornerAdvance:
        if (obs.allBlack) {
          startEncL = encL;
          startEncR = encR;
          enter(State::IntersectionAdvance, now);
          command(cfg.curvePwm, cfg.curvePwm);
          break;
        }
        // Bring the drive axle toward the corner before rotating. The sensor
        // row is ahead of it; spinning at the first edge would cut the corner.
        command(cfg.curvePwm, cfg.curvePwm);
        if (obs.ambiguous) {
          startRecovery(now);
        } else if (distance(encL, encR) >= cfg.cornerAdvanceCm ||
                   now - stateSince >= cfg.cornerAdvanceTimeoutMs) {
          if (canTrackWithoutPivot()) startTracking(now, dt, encL, encR);
          else {
            enter(State::Turn, now);
            pivot();
          }
        }
        break;

      case State::Turn:
        if (obs.allBlack) {
          startEncL = encL;
          startEncR = encR;
          enter(State::IntersectionAdvance, now);
          command(cfg.curvePwm, cfg.curvePwm);
          break;
        }
        if (canTrackWithoutPivot()) {
          startTracking(now, dt, encL, encR);
        } else if (obs.ambiguous || now - stateSince >= cfg.turnTimeoutMs) {
          startRecovery(now);
        } else {
          if (obs.visible) {
            // A newly visible edge can be on the opposite side of the robot.
            // Follow that side now, and retain it if the line is lost again.
            turnDirection = obs.position < 0 ? -1 : 1;
            lastPosition = obs.position;
          }
          pivot();
        }
        break;

      case State::IntersectionAdvance:
        // วิ่งตรงข้ามเส้น 5 จุดดำ
        command(cfg.curvePwm, cfg.curvePwm);
        if (obs.allBlack) {
          // ถ้ายังเจอ 5 จุดดำต่อเนื่องเกินระยะทางหรือเวลาที่กำหนด -> เส้นชัย ให้หยุดทันที
          if (distance(encL, encR) >= cfg.intersectionAdvanceCm ||
              now - stateSince >= cfg.intersectionAdvanceTimeoutMs) {
            stop(Reason::FinishLine);
          }
          break;
        }

        // หลุดจากแถบ 5 จุดดำแล้ว:
        if ((obs.mask & 0x04) != 0 && canTrackWithoutPivot()) {
          // ถ้าเจอเส้นตรงกลาง A3 ให้เดินหน้าเกาะเส้นต่อ
          startTracking(now, dt, encL, encR);
        } else if ((obs.mask & 0x03) != 0 && (obs.mask & 0x18) == 0) {
          // ถ้าไม่มีเส้นตรงกลาง แต่เจอฝั่งซ้าย (A5 หรือ A4) -> เลี้ยวซ้าย
          turnDirection = -1;
          lastPosition = -1.5f;
          enter(State::Turn, now);
          pivot();
        } else if ((obs.mask & 0x18) != 0 && (obs.mask & 0x03) == 0) {
          // ถ้าไม่มีเส้นตรงกลาง แต่เจอฝั่งขวา (A2 หรือ A1) -> เลี้ยวขวา
          turnDirection = 1;
          lastPosition = 1.5f;
          enter(State::Turn, now);
          pivot();
        } else if ((obs.mask & 0x03) != 0 || (obs.mask & 0x18) != 0) {
          // เจอเส้นทั้งสองฝั่ง (ทางแยกตัว T) -> เลี้ยวตามแนวเดิม หรือเลี้ยวซ้าย
          turnDirection = lastPosition > 0.1f ? 1 : -1;
          lastPosition = float(turnDirection) * 1.5f;
          enter(State::Turn, now);
          pivot();
        } else if (distance(encL, encR) >= cfg.intersectionAdvanceCm ||
                   now - stateSince >= cfg.intersectionAdvanceTimeoutMs) {
          // หลุดจากเส้น 5 จุดแล้วและไม่พบเส้นต่อเลยจนหมดระยะ -> เข้าโหมดกู้คืน (Recovery)
          startRecovery(now);
        }
        break;

      case State::RecoveryBrake:
        drive_.brake = true;
        if (now - stateSince >= cfg.brakeMs) {
          if (canTrackWithoutPivot()) startTracking(now, dt, encL, encR);
          else if (!turnSearchDone && turnDirection != 0) {
            turnSearchDone = true;
            enter(State::Turn, now);
            pivot();
          } else {
            startEncL = encL;
            startEncR = encR;
            enter(State::Reverse, now);
            command(-cfg.reversePwm, -cfg.reversePwm);
          }
        }
        break;

      case State::Reverse:
        if (obs.visible && !obs.ambiguous) {
          turnDirection = obs.position < -0.15f ? -1 :
                          (obs.position > 0.15f ? 1 : turnDirection);
          enter(State::AlignBrake, now);
          drive_.brake = true;
        } else if (now - stateSince >= cfg.reverseTimeoutMs ||
                   distance(encL, encR) >= cfg.reverseMaxCm) {
          fault(Reason::LineNotFound);
        } else {
          command(-cfg.reversePwm, -cfg.reversePwm);
        }
        break;

      case State::AlignBrake:
        drive_.brake = true;
        if (now - stateSince >= cfg.brakeMs) {
          enter(State::Align, now);
          alignToLine(now, dt, encL, encR);
        }
        break;

      case State::Align:
        alignToLine(now, dt, encL, encR);
        break;

      default: break;
    }
  }

 private:
  Config cfg;
  State state_ = State::Countdown;
  Reason reason_ = Reason::None;
  Drive drive_;
  Observation obs;
  uint32_t stateSince = 0, lastTick = 0;
  int strength_[SENSOR_COUNT] = {};
  bool straight = false, missing = false, recoveryActive = false, turnSearchDone = false;
  uint32_t straightSince = 0, missingSince = 0, recoverySince = 0;
  uint32_t startEncL = 0, startEncR = 0;
  float lastPosition = 0, lastError = 0, derivative = 0, basePwm = 0;
  int turnDirection = 0;

  static bool held(bool condition, uint32_t now, uint32_t duration,
                   bool& active, uint32_t& since) {
    if (!condition) { active = false; return false; }
    if (!active) { active = true; since = now; }
    return now - since >= duration;
  }

  void enter(State next, uint32_t now) {
    state_ = next;
    stateSince = now;
    straight = missing = false;
  }

  void startTracking(uint32_t now, float dt, uint32_t encL, uint32_t encR) {
    enter(State::Follow, now);
    recoveryActive = false;
    lastPosition = lastError = obs.position;
    derivative = 0;
    basePwm = float(cfg.curvePwm);
    trackVisibleLine(now, dt, encL, encR);
  }

  bool canTrackWithoutPivot() const {
    return obs.visible && !obs.ambiguous && !obs.allBlack && obs.corner == 0 &&
           fabsf(obs.position) < 1.65f;
  }

  void trackVisibleLine(uint32_t now, float dt, uint32_t encL, uint32_t encR) {
    lastPosition = obs.position;
    if (obs.corner != 0) {
      turnDirection = obs.corner;
      startEncL = encL;
      startEncR = encR;
      enter(State::CornerAdvance, now);
      command(cfg.curvePwm, cfg.curvePwm);
    } else if (fabsf(obs.position) >= 1.65f) {
      turnDirection = obs.position < 0 ? -1 : 1;
      enter(State::Turn, now);
      pivot();
    } else {
      follow(now, dt);
    }
  }

  void alignToLine(uint32_t now, float dt, uint32_t encL, uint32_t encR) {
    if (canTrackWithoutPivot()) {
      startTracking(now, dt, encL, encR);
    } else if (obs.ambiguous || now - stateSince >= cfg.turnTimeoutMs) {
      fault(Reason::LineNotFound);
    } else {
      if (obs.visible && fabsf(obs.position) > 0.15f)
        turnDirection = obs.position < 0 ? -1 : 1;
      if (turnDirection == 0) {
        // No evidence of a direction: do not pick a neighboring line.
        drive_.brake = true;
      } else {
        pivot();
      }
    }
  }

  void startRecovery(uint32_t now) {
    if (!recoveryActive) { recoveryActive = true; recoverySince = now; }
    if (state_ == State::Turn) turnSearchDone = true;
    else turnSearchDone = false;
    if (fabsf(lastPosition) > 0.15f) turnDirection = lastPosition < 0 ? -1 : 1;
    enter(State::RecoveryBrake, now);
    drive_ = Drive();
    drive_.brake = true;
  }

  void command(int left, int right) {
    drive_.left = int(clampFloat(float(left), -65535.0f, 65535.0f));
    drive_.right = int(clampFloat(float(right) * cfg.rightTrim, -65535.0f, 65535.0f));
    drive_.brake = true;
  }

  void pivot() {
    // Negative error: line is LEFT -> left wheel reverses, right advances.
    // Center rotation reduces the forward sweep toward an adjacent line.
    if (turnDirection < 0) command(-cfg.turnPwm, cfg.turnPwm);
    else command(cfg.turnPwm, -cfg.turnPwm);
  }

  float distance(uint32_t encL, uint32_t encR) const {
    // Single-channel encoders count rotation magnitude, including reversing.
    return (float(uint32_t(encL - startEncL)) + float(uint32_t(encR - startEncR))) *
           0.5f * cfg.cmPerPulse;
  }

  Observation observe(const int raw[SENSOR_COUNT]) {
    Observation result;
    struct Group { int first, last, weight; float position; } groups[SENSOR_COUNT];
    int count = 0;
    for (int i = 0; i < SENSOR_COUNT; ++i) {
      int span = cfg.blackRaw[i] - cfg.woodRaw[i];
      strength_[i] = int(clampFloat(1000.0f * (raw[i] - cfg.woodRaw[i]) / span, 0, 1000));
      if (strength_[i] < cfg.lineThreshold) continue;
      result.mask |= uint8_t(1U << i);
      if (count == 0 || groups[count - 1].last != i - 1) {
        groups[count].first = i;
        groups[count].last = i;
        groups[count].weight = 0;
        groups[count].position = 0;
        ++count;
      }
      Group& g = groups[count - 1];
      g.last = i;
      g.weight += strength_[i];
      g.position += float(i - 2) * strength_[i];
    }
    if (count == 0) return result;
    if (result.mask == 0x1f) {
      result.visible = true;
      result.allBlack = true;
      result.position = 0.0f;
      return result;
    }
    int best = 0;
    float bestDistance = 100, secondDistance = 100;
    for (int i = 0; i < count; ++i) {
      groups[i].position /= groups[i].weight;
      float d = fabsf(groups[i].position - lastPosition);
      if (d < bestDistance) { secondDistance = bestDistance; bestDistance = d; best = i; }
      else if (d < secondDistance) secondDistance = d;
    }
    // หากเซนเซอร์กลาง (A3) เจอดำ และหุ่นกำลังวิ่งเกาะกลาง ให้ยึดกลุ่มกลางเป็นหลักเสมอ
    if ((result.mask & 0x04) != 0 && fabsf(lastPosition) < 1.2f) {
      for (int i = 0; i < count; ++i) {
        if (groups[i].first <= 2 && groups[i].last >= 2) {
          best = i;
          bestDistance = fabsf(groups[i].position - lastPosition);
          secondDistance = 100.0f;
          break;
        }
      }
    }
    // Never average separated black lines into a fictitious center position.
    if (count > 1 && secondDistance - bestDistance < 0.6f) {
      result.ambiguous = true;
      return result;
    }
    const Group& g = groups[best];
    result.visible = true;
    result.position = g.position;
    result.centered = g.first <= 2 && g.last >= 2 && fabsf(g.position) <= 0.45f;
    // ตรวจจับมุมฉากซ้าย 3 หรือ 4 จุด: A5..A3 (g.first=0, g.last=2) หรือ A5..A2 (g.first=0, g.last=3)
    if (g.first == 0 && (g.last == 2 || g.last == 3)) result.corner = -1;
    // ตรวจจับมุมฉากขวา 3 หรือ 4 จุด: A3..A1 (g.first=2, g.last=4) หรือ A4..A1 (g.first=1, g.last=4)
    if ((g.first == 2 || g.first == 1) && g.last == 4) result.corner = 1;
    return result;
  }

  void follow(uint32_t now, float dt) {
    bool fast = held(fabsf(obs.position) < 0.35f, now, cfg.straightStableMs,
                     straight, straightSince);
    float target = float(fast ? cfg.cruisePwm : cfg.curvePwm);
    if (basePwm > target) basePwm = target;
    else basePwm = clampFloat(basePwm + cfg.accelerationPwmPerSec * dt, 0, target);
    float rate = clampFloat((obs.position - lastError) / dt, -40, 40);
    derivative += 0.25f * (rate - derivative);
    float correction = cfg.steeringKp * obs.position + cfg.steeringKd * derivative;
    lastError = obs.position;
    command(int(clampFloat(basePwm + correction, 0, 65535)),
            int(clampFloat(basePwm - correction, 0, 65535)));
  }
};

inline const char* stateName(State state) {
  switch (state) {
    case State::Countdown: return "COUNTDOWN";
    case State::Follow: return "FOLLOW";
    case State::CornerAdvance: return "CORNER_ADVANCE";
    case State::Turn: return "TURN";
    case State::IntersectionAdvance: return "INTERSECTION_ADVANCE";
    case State::RecoveryBrake: return "RECOVERY_BRAKE";
    case State::Reverse: return "REVERSE";
    case State::AlignBrake: return "ALIGN_BRAKE";
    case State::Align: return "ALIGN";
    case State::Stopped: return "STOPPED";
    case State::Fault: return "FAULT";
  }
  return "UNKNOWN";
}

inline const char* reasonName(Reason reason) {
  switch (reason) {
    case Reason::None: return "NONE";
    case Reason::BleStop: return "BLE_STOP";
    case Reason::SerialStop: return "SERIAL_STOP";
    case Reason::InvalidSensorConfig: return "INVALID_SENSOR_CONFIG";
    case Reason::LineNotFound: return "LINE_NOT_FOUND";
    case Reason::ControlOverrun: return "CONTROL_OVERRUN";
    case Reason::BleUnavailable: return "BLE_UNAVAILABLE";
    case Reason::FinishLine: return "FINISH_LINE";
  }
  return "UNKNOWN";
}

} // namespace linebot
