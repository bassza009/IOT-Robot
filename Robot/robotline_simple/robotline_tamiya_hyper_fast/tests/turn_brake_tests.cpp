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
void braked() {
  CHECK(leftPwm == 0 && rightPwm == 0);
  CHECK(direction[6] == HIGH && direction[7] == HIGH);
  CHECK(direction[9] == HIGH && direction[8] == HIGH);
  CHECK(pwm[5] == 255 && pwm[10] == 255);
}
void leftTurn() { CHECK(leftPwm < 0 && rightPwm > 0); }
void rightTurn() { CHECK(leftPwm > 0 && rightPwm < 0); }

int main(int argc, char** argv) {
  CHECK(argc == 2);
  const char* test = argv[1];
  if (std::strcmp(test, "turn_brake_wrap") == 0) fakeTime = UINT32_MAX - 20;
  setup();
  sample(4); CHECK(leftPwm > 0 && rightPwm > 0);

  if (std::strcmp(test, "corner_brake") == 0 ||
      std::strcmp(test, "turn_brake_wrap") == 0) {
    sample(1); braked(); CHECK(std::strcmp(state, "TURN_BRAKE") == 0);
    sample(1, 39); braked();
    sample(1, 1); leftTurn();
    sample(1, 40); leftTurn(); // One brake on entry, not on every pivot update.
  } else if (std::strcmp(test, "turn_reversal_brake") == 0) {
    sample(1); sample(1, 40); leftTurn();
    sample(16); braked(); sample(16, 39); braked();
    sample(16, 1); rightTurn();
  } else if (std::strcmp(test, "turn_brake_latest_side") == 0) {
    sample(1); braked();
    sample(16, 20); braked(); CHECK(raw[4] == 900 && raw[0] == 3000);
    sample(16, 20); rightTurn(); // Use latest side without restarting the hold.
  } else if (std::strcmp(test, "turn_brake_center") == 0) {
    sample(1); braked(); sample(4, 20); braked();
    sample(4, 20); CHECK(leftPwm > 0 && rightPwm > 0);
    sample(1); braked(); // A new corner after normal tracking gets a new brake.
  } else if (std::strcmp(test, "search_brake") == 0) {
    sample(2); sample(0); braked();
    sample(0, 40); leftTurn(); CHECK(std::strcmp(state, "SEARCH") == 0);
  } else if (std::strcmp(test, "lost_during_turn_brake") == 0) {
    sample(1); braked(); sample(0, 20); braked();
    sample(0, 20); leftTurn(); CHECK(std::strcmp(state, "SEARCH") == 0);
  } else if (std::strcmp(test, "stop_during_turn_brake") == 0) {
    sample(1); braked(); Serial.input = "STOP"; loop();
    sample(16, 100); braked(); CHECK(stopped);
    CHECK(std::strcmp(state, "SERIAL_STOP") == 0);
  } else if (std::strcmp(test, "turn_brake_disabled") == 0) {
    sample(1); leftTurn(); sample(16); rightTurn();
  } else {
    CHECK(false);
  }
  std::printf("PASS %s\n", test);
}
