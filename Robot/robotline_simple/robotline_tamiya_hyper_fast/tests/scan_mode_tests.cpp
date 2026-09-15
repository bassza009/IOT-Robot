#include "../robotline_tamiya_hyper_fast.ino"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define CHECK(test) do { if (!(test)) { \
  std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #test); std::exit(1); \
} } while (0)

int main(int argc, char** argv) {
  CHECK(argc == 2);
  setup();
  if (std::strcmp(argv[1], "settling") == 0) {
    adcSequence = {3000, 900}; // The first conversion still shows the floor.
    CHECK(readSensor(A5) == 900);
    CHECK(adcReads == 2);
  } else if (std::strcmp(argv[1], "median") == 0) {
    adcSequence = {123, 900, 3000, 950}; // Discard, then reject one floor spike.
    CHECK(readSensor(A5) == 950);
    CHECK(adcReads == 4);
    adcSequence = {123, 950, 900, 3000};
    CHECK(readSensor(A5) == 950);
    adcSequence = {123, 3000, 950, 900};
    CHECK(readSensor(A5) == 950);
  } else if (std::strcmp(argv[1], "timed") == 0) {
    for (int pin : {A5, A4, A3, A2, A1}) adc[pin] = 3000;
    adc[A3] = 900;
    loop(); CHECK(leftPwm == 0);
    fakeTime += 1; loop(); CHECK(leftPwm == 0);
    fakeTime += 1; loop(); CHECK(leftPwm > 0 && rightPwm > 0);
    int count = adcReads;
    Serial.input = "STOP"; loop();
    CHECK(stopped && leftPwm == 0 && rightPwm == 0);
    CHECK(adcReads == count); // STOP still works between timed samples.
  } else {
    CHECK(false);
  }
  std::printf("PASS %s\n", argv[1]);
}
