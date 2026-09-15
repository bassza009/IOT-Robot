#include <Arduino.h>
#include <stdio.h>

// UNO R4 WiFi. Physical order: left -> right, same as robotline_simple.
const int SENSOR_PINS[5] = {A5, A4, A3, A2, A1};
const int LEFT_PWM = 5, LEFT_A = 6, LEFT_B = 7;
const int RIGHT_PWM = 10, RIGHT_A = 9, RIGHT_B = 8;
const unsigned long REPORT_MS = 100;

int raw[5] = {};

void setup() {
  // Disable both motor bridges before setting their direction pins.
  // This diagnostic never enables either motor.
  analogWriteResolution(8);
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);
  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, 0);
  pinMode(LEFT_A, OUTPUT);
  pinMode(LEFT_B, OUTPUT);
  pinMode(RIGHT_A, OUTPUT);
  pinMode(RIGHT_B, OUTPUT);
  digitalWrite(LEFT_A, LOW);
  digitalWrite(LEFT_B, LOW);
  digitalWrite(RIGHT_A, LOW);
  digitalWrite(RIGHT_B, LOW);

  analogReadResolution(12); // Same 0..4095 scale as the line follower.
  for (int i = 0; i < 5; ++i) pinMode(SENSOR_PINS[i], INPUT);
  Serial.begin(115200);
}

void loop() {
  // One fresh reading per sensor, every loop. No fixed sampling delay.
  for (int i = 0; i < 5; ++i) raw[i] = analogRead(SENSOR_PINS[i]);

  static char report[96];
  static size_t length = 0, sent = 0;
  static uint32_t lastReport = 0;
  if (!Serial) {
    length = sent = 0;
    return;
  }

  uint32_t now = millis();
  if (sent == length && now - lastReport >= REPORT_MS) {
    lastReport = now;
    // Named numeric fields work in Serial Monitor and Serial Plotter.
    int n = snprintf(report, sizeof(report),
                     "A5:%d\tA4:%d\tA3:%d\tA2:%d\tA1:%d\n",
                     raw[0], raw[1], raw[2], raw[3], raw[4]);
    length = n > 0 ? (size_t(n) < sizeof(report) ? size_t(n) : sizeof(report) - 1) : 0;
    sent = 0;
  }

  // Keep sampling even when the Serial output buffer is full.
  int room = Serial.availableForWrite();
  size_t count = length - sent;
  if (room > 0 && count > 0) {
    if (count > size_t(room)) count = size_t(room);
    sent += Serial.write(reinterpret_cast<const uint8_t*>(report) + sent, count);
  }
}
