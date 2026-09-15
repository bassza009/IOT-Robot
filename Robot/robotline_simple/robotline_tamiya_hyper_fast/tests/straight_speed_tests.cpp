#include "../robotline_tamiya_hyper_fast.ino"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define CHECK(test) do { if (!(test)) { \
  std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #test); std::exit(1); \
} } while (0)

void sample(int mask, uint32_t ms = 1) {
  const int pins[] = {A5, A4, A3, A2, A1};
  for (int i = 0; i < 5; ++i) adc[pins[i]] = mask & (1 << i) ? 900 : 3000;
  fakeTime += ms;
  loop();
}
void hold(int mask, uint32_t ms) {
  while (ms > 0) {
    uint32_t step = ms > 50 ? 50 : ms;
    sample(mask, step);
    ms -= step;
  }
}
void speed(int left, int right) { CHECK(leftPwm == left && rightPwm == right); }
void braked() {
  speed(0, 0);
  CHECK(pwm[5] == 255 && pwm[10] == 255);
  CHECK(direction[6] == HIGH && direction[7] == HIGH);
  CHECK(direction[9] == HIGH && direction[8] == HIGH);
}

int main(int argc, char** argv) {
  CHECK(argc == 2);
  const char* test = argv[1];
  if (std::strcmp(test, "straight_wrap") == 0) fakeTime = UINT32_MAX - 500;
  setup();
  sample(4);

  if (std::strcmp(test, "straight_profile") == 0 ||
      std::strcmp(test, "straight_wrap") == 0) {
    speed(120, 101);
    hold(4, 500); speed(140, 118);
    hold(4, 500); speed(160, 134);
    hold(4, 2999); speed(160, 134);
    hold(4, 1); speed(160, 134);
    hold(4, 500); speed(140, 118);
    hold(4, 500); speed(120, 101);
    hold(4, 500); speed(140, 118); // Second cycle starts without a corner.
    hold(4, 500); speed(160, 134);
    hold(4, 4000); speed(120, 101);
  } else if (std::strcmp(test, "straight_missed_boundary") == 0) {
    hold(4, 4990);
    sample(4, 110); speed(124, 104); // 100 ms into cycle 2, not a new t=0.
  } else if (std::strcmp(test, "straight_curve_reset") == 0) {
    hold(4, 1500); speed(160, 134);
    sample(2); speed(40, 168); // Ordinary left correction uses the slow base.
    sample(4); speed(120, 101);
    hold(4, 500); speed(140, 118);
  } else if (std::strcmp(test, "straight_corner_reset") == 0) {
    hold(4, 1500); sample(1); braked();
    sample(1, 40); CHECK(leftPwm < 0 && rightPwm > 0);
    sample(4); speed(120, 101);
    hold(4, 500); speed(140, 118);
  } else if (std::strcmp(test, "straight_cross_reset") == 0) {
    hold(4, 1500); sample(31); speed(110, 92);
    hold(31, 100); sample(4); speed(120, 101);
  } else if (std::strcmp(test, "straight_recovery_reset") == 0) {
    hold(4, 1500); sample(17); braked();
    hold(17, 80); CHECK(leftPwm < 0 && rightPwm < 0);
    sample(4); braked(); hold(4, 80); speed(120, 101);
    hold(4, 500); speed(140, 118);
  } else if (std::strcmp(test, "straight_pattern_change") == 0) {
    hold(4, 1000); sample(14); speed(160, 134);
    hold(14, 500); sample(4); speed(160, 134);
  } else if (std::strcmp(test, "straight_loss_reset") == 0) {
    sample(2); sample(4); hold(4, 1500);
    sample(0); braked(); sample(0, 40);
    CHECK(leftPwm < 0 && rightPwm > 0);
    sample(4); speed(120, 101);
  } else if (std::strcmp(test, "straight_serial_busy") == 0) {
    Serial.space = 0;
    hold(4, 500); speed(140, 118);
    hold(4, 3500); speed(160, 134);
    hold(4, 1000); speed(120, 101);
  } else if (std::strncmp(test, "straight_stop_", 14) == 0) {
    unsigned elapsed = std::strcmp(test, "straight_stop_up") == 0 ? 500 :
                       std::strcmp(test, "straight_stop_hold") == 0 ? 2500 : 4500;
    hold(4, elapsed);
    Serial.input = "STOP"; loop(); braked();
    hold(4, 6000); braked(); CHECK(stopped);
  } else {
    CHECK(false);
  }
  std::printf("PASS %s\n", test);
}
