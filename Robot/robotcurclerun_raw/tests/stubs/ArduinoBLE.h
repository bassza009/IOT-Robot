#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <string>

const int A1 = 15, A2 = 16, A3 = 17, A4 = 18, A5 = 19;
const int HIGH = 1, LOW = 0, OUTPUT = 1, INPUT = 0, INPUT_PULLUP = 2;
const int LED_BUILTIN = 13, RISING = 1;
const int BLENotify = 1, BLEWrite = 2, BLEWriteWithoutResponse = 4, BLEWritten = 1;
static uint32_t fakeTime = 0;
static int fakeAnalog[32], fakePwm[32], fakeDigital[32], fakeMode[32];
static int fakeWriteResolution = 0, fakeReadResolution = 0;
static int fakePwmWrites = 0, fakeZeroPwmWrites = 0;
inline uint32_t millis() { return fakeTime; }
inline void pinMode(int pin, int mode) { fakeMode[pin] = mode; }
inline void digitalWrite(int pin, int value) { fakeDigital[pin] = value; }
inline void analogWrite(int pin, int value) {
  fakePwm[pin] = value; ++fakePwmWrites;
  if (value == 0) ++fakeZeroPwmWrites;
}
inline int analogRead(int pin) { return fakeAnalog[pin]; }
inline void analogReadResolution(int bits) { fakeReadResolution = bits; }
inline void analogWriteResolution(int bits) { fakeWriteResolution = bits; }
inline int digitalPinToInterrupt(int pin) { return pin; }
inline void attachInterrupt(int, void (*)(), int) {}
inline void noInterrupts() {}
inline void interrupts() {}

struct FakeSerial {
  const char* input = "";
  std::string output;
  void begin(int) {}
  operator bool() const { return true; }
  int available() const { return int(strlen(input)); }
  int read() { return *input ? *input++ : -1; }
  int availableForWrite() const { return 64; }
  size_t write(const uint8_t* value, size_t n) {
    output.append((const char*)value, n);
    return n;
  }
};
static FakeSerial Serial;

class BLEDevice {};
class BLECharacteristic;
typedef void (*BleCallback)(BLEDevice, BLECharacteristic);
class BLECharacteristic {
 public:
  BLECharacteristic(const char*, int, int) {}
  bool subscribed() const { return subscribed_; }
  int valueLength() const { return int(strlen(data)); }
  const uint8_t* value() const { return (const uint8_t*)data; }
  int writeValue(const uint8_t* value, size_t n) {
    ++notifications;
    output.append((const char*)value, n);
    return 1;
  }
  void setEventHandler(int, BleCallback cb) { callback = cb; }
  void simulateWrite(const char* text) {
    strncpy(data, text, sizeof(data) - 1);
    data[sizeof(data) - 1] = 0;
    if (callback) callback(BLEDevice(), *this);
  }
  bool subscribed_ = true;
  int notifications = 0;
  std::string output;
 private:
  char data[65] = {};
  BleCallback callback = nullptr;
};
class BLEService {
 public:
  explicit BLEService(const char*) {}
  void addCharacteristic(BLECharacteristic&) {}
};
struct FakeBLE {
  bool beginOK = true;
  uint32_t beginDelayMs = 0;
  void (*onAdvertise)() = nullptr;
  int polls = 0;
  bool begin() { fakeTime += beginDelayMs; return beginOK; }
  void setLocalName(const char*) {}
  void setDeviceName(const char*) {}
  void setAdvertisedService(BLEService&) {}
  void addService(BLEService&) {}
  void advertise() { if (onAdvertise) onAdvertise(); }
  void poll() { ++polls; }
};
static FakeBLE BLE;
