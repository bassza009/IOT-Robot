#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <cassert>
#include <deque>

const int A1 = 15, A2 = 16, A3 = 17, A4 = 18, A5 = 19;
const int HIGH = 1, LOW = 0, INPUT = 0, OUTPUT = 1, LED_BUILTIN = 13;
static uint32_t fakeTime = 0;
static uint32_t subMillis = 0, adcReadUs = 0;
static int adcReads = 0;
static std::deque<int> adcSequence;
static bool stopDuringRead = false;
static int adc[32], pwm[32], direction[32], mode[32];
static int adcBits = 0, pwmBits = 0, zeroWrites = 0;
inline uint32_t millis() { return fakeTime; }
inline uint32_t micros() { return fakeTime * 1000u + subMillis; }
inline void advanceUs(uint32_t elapsed) {
  subMillis += elapsed;
  fakeTime += subMillis / 1000;
  subMillis %= 1000;
}
inline void pinMode(int pin, int value) { mode[pin] = value; }
inline void analogReadResolution(int bits) { adcBits = bits; }
inline void analogWriteResolution(int bits) { pwmBits = bits; }
int analogRead(int pin);
inline void analogWrite(int pin, int value) {
  assert(value >= 0 && value <= 255);
  pwm[pin] = value;
  if (value == 0) ++zeroWrites;
}
inline void digitalWrite(int pin, int value) {
  // Direction changes must happen with that motor's enable disabled.
  if (pin >= 6 && pin <= 9 && direction[pin] != value)
    assert(pwm[pin <= 7 ? 5 : 10] == 0);
  direction[pin] = value;
}
struct FakeSerial {
  std::string input, output;
  bool connected = true;
  int space = 16;
  void begin(int) {}
  operator bool() const { return connected; }
  int available() const { return int(input.size()); }
  int read() {
    int ch = static_cast<unsigned char>(input.front());
    input.erase(0, 1);
    return ch;
  }
  int availableForWrite() const { return space; }
  size_t write(const uint8_t* bytes, size_t length) {
    assert(connected && length <= size_t(space));
    output.append(reinterpret_cast<const char*>(bytes), length);
    return length;
  }
};
static FakeSerial Serial;
inline int analogRead(int pin) {
  ++adcReads;
  advanceUs(adcReadUs);
  if (stopDuringRead) { Serial.input = "STOP"; stopDuringRead = false; }
  if (adcSequence.empty()) return adc[pin];
  int value = adcSequence.front();
  adcSequence.pop_front();
  return value;
}
