// Real sketch; simulated ADC time makes the removed scan delay observable.
#include "../robotline_tamiya_hyper_fast.ino"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define CHECK(test) do { if (!(test)) { \
  std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #test); std::exit(1); \
} } while (0)

void sensors(int mask) {
  const int pins[] = {A5, A4, A3, A2, A1};
  for (int i = 0; i < 5; ++i) adc[pins[i]] = mask & (1 << i) ? 900 : 3000;
}

int main(int argc, char** argv) {
  CHECK(argc == 2);
  const char* test = argv[1];
  if (std::strcmp(test, "micro_wrap") == 0) {
    fakeTime = UINT32_MAX / 1000;
    subMillis = 250;
  }
  setup();
  if (std::strcmp(test, "fast_reaction") == 0) {
    adcReadUs = 10;
    sensors(4); loop(); CHECK(leftPwm > 0 && rightPwm > 0);
    // Ordinary steering still reacts before the old 5 ms sample deadline.
    int straightLeft = leftPwm, straightRight = rightPwm;
    sensors(2); advanceUs(100); loop();
    CHECK(leftPwm < straightLeft && rightPwm > straightRight);
    sensors(8); advanceUs(100); loop();
    CHECK(leftPwm > straightLeft && rightPwm < straightRight);
    CHECK(millis() == 0);
  } else if (std::strcmp(test, "scan_work") == 0) {
    sensors(4);
    adcReadUs = 10;
    uint32_t start = micros();
    readLine();
    CHECK(lineMask == 4 && lineError == 0);
    CHECK(adcReads == 5 && micros() - start == 50);
  } else if (std::strcmp(test, "stop_during_scan") == 0) {
    sensors(4); fakeTime += 5; loop(); CHECK(leftPwm > 0);
    stopDuringRead = true;
    sensors(1); fakeTime += 5; loop();
    CHECK(stopped && leftPwm == 0 && rightPwm == 0);
    CHECK(std::strcmp(state, "SERIAL_STOP") == 0);
  } else if (std::strcmp(test, "timing_report") == 0 ||
             std::strcmp(test, "micro_wrap") == 0) {
    sensors(4); adcReadUs = 10;
    for (int i = 0; i < 2400; ++i) loop();
    CHECK(leftPwm > 0 && rightPwm > 0 && !stopped);
    CHECK(Serial.output.find("SCAN_US=50") != std::string::npos);
    CHECK(Serial.output.find("GAP_US=50") != std::string::npos);
    CHECK(Serial.output.find('\n') != std::string::npos);
  } else if (std::strcmp(test, "adc_overrun") == 0) {
    sensors(4); fakeTime += 5; loop(); CHECK(leftPwm > 0);
    adcReadUs = 40000; loop(); // 200 ms spent scanning while motors were running.
    CHECK(stopped && leftPwm == 0 && rightPwm == 0);
    CHECK(std::strcmp(state, "CONTROL_OVERRUN") == 0);
  } else {
    CHECK(false);
  }
  std::printf("PASS %s\n", test);
}
