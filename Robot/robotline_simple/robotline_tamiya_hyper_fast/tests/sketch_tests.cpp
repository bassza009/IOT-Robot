// Test the actual setup/loop. Only the Arduino hardware is simulated.
#include "../robotline_tamiya_hyper_fast.ino"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static int checks = 0;
#define CHECK(test) do { ++checks; if (!(test)) { \
  std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #test); std::exit(1); \
} } while (0)

int wheel(int enable, int a, int b) {
  if (direction[a] == direction[b]) return 0;
  return direction[a] == LOW ? pwm[enable] : -pwm[enable];
}
int left() { return wheel(5, 6, 7); }
int right() { return wheel(10, 9, 8); }
void sample(int mask, uint32_t elapsed = 5) {
  // Independently specified physical order: A5, A4, A3, A2, A1.
  const int pins[] = {A5, A4, A3, A2, A1};
  for (int i = 0; i < 5; ++i) adc[pins[i]] = mask & (1 << i) ? 900 : 3000;
  fakeTime += elapsed;
  loop();
}
void run(int mask, unsigned ms) {
  for (unsigned i = 0; i < ms; i += 5) sample(mask);
}
void turnSample(int mask) {
  sample(mask);
  run(mask, TURN_BRAKE_MS);
}
void braked() {
  CHECK(left() == 0 && right() == 0);
  CHECK(direction[6] == HIGH && direction[7] == HIGH);
  CHECK(direction[9] == HIGH && direction[8] == HIGH);
  CHECK(pwm[5] == 255 && pwm[10] == 255);
}

int main(int argc, char** argv) {
  CHECK(argc == 2);
  const char* test = argv[1];
  if (std::strcmp(test, "wrap") == 0 || std::strcmp(test, "reverse_wrap") == 0)
    fakeTime = UINT32_MAX - 100;
  setup();
  CHECK(left() == 0 && right() == 0);
  CHECK(adcBits == 12 && pwmBits == 8);

  if (std::strcmp(test, "steering") == 0) {
    sample(4); CHECK(left() > 0 && right() > 0);
    int straightL = left(), straightR = right(), disables = zeroWrites;
    sample(2); CHECK(left() < straightL && right() > straightR);
    int strongLeft = left();
    sample(6); CHECK(left() > strongLeft && left() < straightL);
    sample(8); CHECK(left() > straightL && right() < straightR);
    sample(12); CHECK(left() > straightL && right() < straightR);
    CHECK(zeroWrites == disables); // Changing forward speed must not cut enable.
    turnSample(1); CHECK(left() < 0 && right() > 0);
    turnSample(16); CHECK(left() > 0 && right() < 0);
    turnSample(7); CHECK(left() < 0 && right() > 0);
    turnSample(28); CHECK(left() > 0 && right() < 0);
    sample(4); CHECK(left() > 0 && right() > 0);
    sample(14); CHECK(left() == straightL && right() == straightR);
  } else if (std::strcmp(test, "stop") == 0) {
    turnSample(1);
    Serial.input = "st"; loop();
    Serial.input = "op"; loop(); // No newline or next sample required.
    braked();
    Serial.input = "START\n";
    run(4, 2000); braked();
    CHECK(Serial.output.find("SERIAL_STOP") != std::string::npos);
    CHECK(Serial.output.find("RAW=") != std::string::npos);
  } else if (std::strcmp(test, "startup_stop") == 0) {
    Serial.input = "STOP";
    sample(4); braked();
  } else if (std::strcmp(test, "loss_left") == 0 ||
             std::strcmp(test, "wrap") == 0) {
    sample(2); turnSample(0);
    CHECK(left() < 0 && right() > 0);
    run(0, 400); CHECK(left() < 0 && right() > 0);
    sample(4); CHECK(left() > 0 && right() > 0);
    sample(8); turnSample(0); CHECK(left() > 0 && right() < 0);
    run(0, SEARCH_MS - TURN_BRAKE_MS); braked();
    run(0, 80); CHECK(left() < 0 && right() < 0);
    run(0, 650); braked();
    run(0, 2000); braked(); // Waiting must not keep backing away indefinitely.
    run(4, 100); CHECK(left() > 0 && right() > 0); // No RESET needed.
  } else if (std::strcmp(test, "unknown_line") == 0) {
    sample(0); braked();
    run(0, 80); CHECK(left() < 0 && right() < 0);
    sample(4); braked();
    run(4, 80); CHECK(left() > 0 && right() > 0);
    CHECK(Serial.output.find("LAST_ERROR=LINE_LOST") != std::string::npos);
  } else if (std::strcmp(test, "lost_center") == 0) {
    sample(4); sample(0); braked();
    run(0, 80); CHECK(left() < 0 && right() < 0);
    run(0, 650); braked();
  } else if (std::strcmp(test, "turn_timeout") == 0) {
    turnSample(1); CHECK(left() < 0 && right() > 0);
    run(1, 1900); CHECK(left() < 0 && right() < 0);
    // The same outer sensor that caused the failed turn is not recovery.
    run(1, 650); braked();
    run(1, 500); braked();
    run(2, 100); CHECK(left() > 0 && right() > left());
  } else if (std::strcmp(test, "turn_recovery") == 0) {
    turnSample(1); run(1, 1900); CHECK(left() < 0 && right() < 0);
    sample(2); braked();
    run(2, 80); CHECK(left() > 0 && right() > left());
  } else if (std::strcmp(test, "black_strip") == 0) {
    sample(4); sample(31); CHECK(left() > 0 && right() > 0);
    run(31, 100); sample(4); CHECK(left() > 0 && right() > 0);
    run(31, 300); braked();
    run(31, 80); CHECK(left() < 0 && right() < 0);
    run(31, 650); braked();
    run(4, 100); CHECK(left() > 0 && right() > 0);
  } else if (std::strcmp(test, "ambiguous") == 0) {
    sample(4); sample(17); braked();
    run(17, 80); CHECK(left() < 0 && right() < 0);
    run(31, 150); CHECK(left() < 0 && right() < 0);
    sample(6); braked();
    run(6, 80); CHECK(left() > 0 && right() > left());
  } else if (std::strcmp(test, "reverse_wrap") == 0) {
    sample(0); run(0, 150); CHECK(left() < 0 && right() < 0);
    run(0, 600); braked(); sample(4); braked();
    run(4, 80); CHECK(left() > 0 && right() > 0);
  } else if (std::strcmp(test, "reverse_flicker") == 0) {
    sample(0); run(0, 200); CHECK(left() < 0 && right() < 0);
    sample(4); braked(); run(0, 80); CHECK(left() < 0 && right() < 0);
    sample(4); braked(); run(0, 80); CHECK(left() < 0 && right() < 0);
    run(0, 500); braked(); sample(4); braked();
    run(0, 100); braked(); // Fleeting line while waiting must not restart driving.
    run(4, 100); CHECK(left() > 0 && right() > 0);
  } else if (std::strcmp(test, "wait_line_stop") == 0) {
    sample(4); sample(0); run(0, 800); braked();
    Serial.input = "STOP"; loop(); braked();
    Serial.input = "START\n"; run(4, 2000); braked();
    CHECK(Serial.output.find("SERIAL_STOP") != std::string::npos);
  } else if (std::strcmp(test, "stop_error_brake") == 0 ||
             std::strcmp(test, "stop_reverse") == 0 ||
             std::strcmp(test, "stop_line_brake") == 0) {
    sample(4); sample(0); braked();
    if (std::strcmp(test, "stop_error_brake") != 0) {
      run(0, 80); CHECK(left() < 0 && right() < 0);
    }
    if (std::strcmp(test, "stop_line_brake") == 0) { sample(4); braked(); }
    Serial.input = "STOP"; loop(); braked();
    Serial.input = "START\n"; run(4, 2000); braked();
    CHECK(Serial.output.find("SERIAL_STOP") != std::string::npos);
  } else if (std::strcmp(test, "reverse_overrun") == 0) {
    sample(0); run(0, 80); CHECK(left() < 0 && right() < 0);
    sample(0, 200); braked(); run(4, 500); braked();
    CHECK(Serial.output.find("CONTROL_OVERRUN") != std::string::npos);
  } else if (std::strcmp(test, "overrun") == 0) {
    sample(4); sample(4, 200); braked();
  } else if (std::strcmp(test, "serial_busy") == 0) {
    Serial.space = 0;
    run(4, 1000); CHECK(left() > 0 && right() > 0);
    Serial.connected = false;
    run(2, 500); CHECK(left() > 0 && right() > left());
    Serial.connected = true; Serial.space = 3;
    run(4, 1000);
    CHECK(Serial.output.find("RAW=") != std::string::npos);
    CHECK(Serial.output.find('\n') != std::string::npos);
    Serial.input = std::string(1000, 'x');
    sample(8); CHECK(left() > right() && Serial.input.size() >= 968);
    Serial.input = "\nSTOP"; loop(); braked();
  } else {
    CHECK(false);
  }
  std::printf("PASS %s (%d checks)\n", test, checks);
}
