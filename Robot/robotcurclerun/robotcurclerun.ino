#include <Wire.h>
#include "Arduino_LED_Matrix.h"
#include "E:\github\IOT-Robot\Robot\animation\animation.h"
#include "Servo.h"
#include <ArduinoBLE.h>

// ============================================================================
// 1. HARDWARE PIN DEFINITIONS
// ============================================================================
// ============================================================================
// 1. PIN CONFIGURATION (ยืนยันผลทดสอบฮาร์ดแวร์จริง 100%)
// ============================================================================
// Right Motor (L298N Channel A)
const int ENA = 10; // มอเตอร์ขวา (PWM ขา 10)
const int IN1 = 8;  // ทิศทางล้อขวา 1 9
const int IN2 = 9;  // ทิศทางล้อขวา 2 8

// Left Motor (L298N Channel B)
const int ENB = 5;  // มอเตอร์ซ้าย (PWM ขา 5)
const int IN3 = 6;  // ทิศทางล้อซ้าย 1 7
const int IN4 = 7;  // ทิศทางล้อซ้าย 2 6

// Servo
const int SV = 12;

// LM393 Speed Encoders
const int ENCODER_L = 3;  // เซนเซอร์นับพัลส์ล้อซ้าย (ขา 3)
const int ENCODER_R = 11; // เซนเซอร์นับพัลส์ล้อขวา (ขา 11)

// Objects
Servo servo;
ArduinoLEDMatrix matrix;
/*this code ทำให้รถวิ่งตรง
// ============================================================================
// 16-BIT SPEED & BALANCE CONFIGURATION (analogWriteResolution = 16-bit: 0 - 65535)
// ============================================================================
int RUN_SPEED = 60500;   // ความเร็ววิ่งตรง 16-bit จุดสมดุลธรรมชาติล้อซ้าย (เหลือ Headroom ถึง 65535 ให้เร่งแซงได้)
int TURN_SPEED = 65535;  // ความเร็วตอนหมุนเลี้ยว 16-bit สูงสุดเต็มพิกัด 100% (ป้องกันมอเตอร์กระตุกเวลาหมุนบนพื้น)
int BRAKE_REVERSE_MS = 35; // สวนกระแสมอเตอร์เพื่อหยุดแรงเฉื่อยสะบัดทันที (ms)
int BRAKE_HOLD_MS = 150;   // ล็อกล้อด้วยระบบ Dynamic Brake (ms)

// Motor Balance Tuning 16-bit
float MOTOR_L_RATIO = 3.0;   // ตัวคูณ PWM ฐานล้อซ้าย (ENA) — ลดค่านี้ = ล้อซ้ายช้าลง
float MOTOR_R_RATIO = 0.995; // ตัวคูณ PWM ฐานล้อขวา (ENB) — ลดค่านี้ = ล้อขวาช้าลง
                              // bisect: 1.0 เบี้ยวขวา, 0.85 เบี้ยวซ้ายเกิน -> 0.925 อยู่กึ่งกลาง
float ENC_TRIM_L = 1.0;      // ตัวคูณค่า pulse ซ้ายก่อนเข้าสูตร error (ไม่กระทบ PWM จริง แค่การอ่านค่า)
float TURN_DEG_SCALE = (360.0 / 180.0) * (360.0 / 202.5); // ตัวชดเชยมุมหมุน/เลี้ยว — เพิ่ม/ลดถ้ามุมจริงขาด/เกิน
float Kp_enc = 750.0;        // ความไวของ loop แก้สมดุลล้อ (จาก encoder) — สูง=ตอบสนองไว แต่ส่ายง่าย
float Ki_enc = 14.0;         // ตัวสะสม error ระยะยาว (จาก encoder) — แก้อาการเบี้ยวสะสมเป็นเส้นโค้ง
float Kp_gyro = 900.0;       // ความไวของ loop แก้สมดุล (จาก gyro, ใช้เมื่อ USE_MPU6050 = true)


*/
// ============================================================================
// 2. ROBOT PHYSICAL GEOMETRY & CALIBRATION
// ============================================================================
const int DISK_SLOTS = 20;                     // 20 holes on encoder disc (1 revolution)
const float WHEEL_DIAMETER_CM = 6.5;           // Wheel diameter: 6.5 cm
const float TRACK_WIDTH_CM = 14.5;             // Distance between left & right wheels: 14.5 cm
const float WHEEL_CIRCUMFERENCE_CM = 3.14159265 * WHEEL_DIAMETER_CM; // ~20.42 cm
const float CM_PER_PULSE = WHEEL_CIRCUMFERENCE_CM / DISK_SLOTS;      // ~1.021 cm per pulse

// ============================================================================
// MANUAL FIXED SPEED CONFIGURATION (16-bit PWM: 0 - 65535)
// ปิดระบบเพิ่มอัตโนมัติ 100% - ผู้ใช้ปรับจูนความเร็วมอเตอร์คงที่แยกซ้าย-ขวาได้โดยตรง
// ============================================================================
int SPEED_L = 52000;   // ความเร็วมอเตอร์ซ้าย (ENB ขา 5) - ถ้าออกขวาให้ลดลง, ถ้าออกซ้ายให้เพิ่มขึ้น
int SPEED_R = 52000;   // ความเร็วมอเตอร์ขวา (ENA ขา 10)
int RUN_SPEED = 52000; // ความเร็วอ้างอิงทั่วไป
int TURN_SPEED = 65535;  // ความเร็วตอนหมุนเลี้ยว 16-bit สูงสุดเต็มพิกัด 100%
int BRAKE_REVERSE_MS = 35; // สวนกระแสมอเตอร์เพื่อหยุดแรงเฉื่อยสะบัดทันที (ms)
int BRAKE_HOLD_MS = 150;   // ล็อกล้อด้วยระบบ Dynamic Brake (ms)
float TURN_DEG_SCALE = (360.0 / 270.0) * (360.0 / 402.5); // สเกลเทียบตามผลวัดจริง 402.5° (= 1.19255)

// ============================================================================
// 3. ENCODER COUNTERS & ISRs
// ============================================================================
volatile unsigned long pulse_count_L = 0;
volatile unsigned long pulse_count_R = 0;

void isr_count_L() { pulse_count_L++; }
void isr_count_R() { pulse_count_R++; }

void resetEncoders() {
  pulse_count_L = 0;
  pulse_count_R = 0;
}

float getRevolutionsL() { return (float)pulse_count_L / DISK_SLOTS; }
float getRevolutionsR() { return (float)pulse_count_R / DISK_SLOTS; }

// ============================================================================
// 4. BLUETOOTH LOW ENERGY (BLE) - NORDIC UART SERVICE (iOS & Android)
// ============================================================================
const bool USE_BLE = true; // เปิดใช้งาน Bluetooth ไร้สายสำหรับ iPhone
BLEService uartService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLECharacteristic txChar("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify, 64);
BLECharacteristic rxChar("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite | BLEWriteWithoutResponse, 64);
bool ble_connected = false;
bool emergency_stop = false;

void sendTelemetry(const String &msg) {
  Serial.println(msg);
  if (USE_BLE && ble_connected && txChar.subscribed()) {
    txChar.writeValue(msg.c_str(), msg.length());
  }
}

void processBLE() {
  if (!USE_BLE) return;
  BLE.poll();
  BLEDevice central = BLE.central();
  if (central && central.connected()) {
    ble_connected = true;
    if (rxChar.written()) {
      int len = rxChar.valueLength();
      const uint8_t* val = rxChar.value();
      String cmd = "";
      for (int i = 0; i < len; i++) cmd += (char)val[i];
      cmd.trim();
      cmd.toUpperCase();
      if (cmd == "STOP") {
        emergency_stop = true;
        analogWrite(ENA, 0);
        analogWrite(ENB, 0);
        sendTelemetry("[BLE CMD] EMERGENCY STOP ACTIVATED!");
      } else if (cmd == "START") {
        emergency_stop = false;
        sendTelemetry("[BLE CMD] START RECEIVED!");
      }
    }
  } else {
    ble_connected = false;
  }
}

// ============================================================================
// 4.1 MPU-6050 GYROSCOPE DRIVER (Auto-Detect & Fallback)
// ============================================================================
// Set to true only when MPU-6050 is physically wired to A4(SDA) and A5(SCL)
// Set to false when using Encoders only (prevents I2C hanging on floating pins!)
const bool USE_MPU6050 = false;

const int MPU_ADDR = 0x68;
bool has_mpu = false;
float gyro_z_offset = 0;
float current_yaw = 0;
unsigned long last_gyro_time = 0;

bool initMPU6050() {
  if (!USE_MPU6050) {
    Serial.println("[IMU] MPU-6050 disabled (USE_MPU6050 = false). Using Encoders for heading & turning.");
    has_mpu = false;
    return false;
  }

  // If enabled, turn on internal pullups and timeout to prevent hanging on floating lines
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  Wire.begin();
  #if defined(WIRE_HAS_TIMEOUT) || defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_UNOR4_MINIMA)
  Wire.setWireTimeout(10000, true); // 10ms timeout with auto-reset to prevent lockup
  #endif

  Wire.beginTransmission(MPU_ADDR);
  byte error = Wire.endTransmission();
  if (error != 0) {
    Serial.println("[IMU] MPU-6050 not detected. Using Encoders for heading & turning.");
    has_mpu = false;
    return false;
  }

  // Wake up MPU-6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  // Set Gyro full scale range to +/- 250 deg/s
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  Serial.println("[IMU] MPU-6050 detected! Calibrating gyro Z (keep robot still)...");
  long sum_z = 0;
  for (int i = 0; i < 200; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x47);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 2, true);
    if (Wire.available() >= 2) {
      int16_t raw_z = (Wire.read() << 8) | Wire.read();
      sum_z += raw_z;
    }
    delay(3);
  }
  gyro_z_offset = (float)sum_z / 200.0;
  current_yaw = 0;
  last_gyro_time = micros();
  has_mpu = true;
  Serial.println("[IMU] Calibration complete! Gyroscope Heading Lock is ACTIVE.");
  return true;
}

void updateYaw() {
  if (!has_mpu) return;

  unsigned long now = micros();
  float dt = (now - last_gyro_time) / 1000000.0;
  last_gyro_time = now;

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x47);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  if (Wire.available() >= 2) {
    int16_t raw_z = (Wire.read() << 8) | Wire.read();
    float rate_z = (raw_z - gyro_z_offset) / 131.0; // 131 LSB/(deg/s)
    if (abs(rate_z) > 0.15) { // Filter sensor noise
      current_yaw += rate_z * dt;
    }
  }
}

void resetYaw() {
  current_yaw = 0;
  last_gyro_time = micros();
}

// ============================================================================
// 5. MOTOR CONTROL PRIMITIVES
// ============================================================================
void stop() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 0);
}

// Active Electronic Brake (L298N Dynamic Brake: short-circuits coils to lock wheels rigid)
void brake(int duration_ms = 150) {
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 65535);
  analogWrite(ENB, 65535);
  if (duration_ms > 0) {
    delay(duration_ms);
  }
}

// Simultaneous Motor Power Application
// แมปสายที่พิสูจน์แล้วว่า Route 1 วิ่งตรง 327/327:
void applyMotorSpeeds(int current_L, int current_R) {
  analogWrite(ENB, current_L); // ENB = current_L
  analogWrite(ENA, current_R); // ENA = current_R
}

// ============================================================================
// 6. HIGH-SPEED NAVIGATION APIs
// ============================================================================

// Forward by distance with direct fixed motor speeds (ปิดระบบปรับเพิ่มอัตโนมัติ วิ่งตรงตามค่า PWM คงที่)
void forward(int speed_motorL, int speed_motorR, float distance_cm = 0) {
  resetEncoders();
  resetYaw();

  // Set motor directions forward
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  // Apply fixed speeds directly (ไม่มีการปรับเพิ่ม/ลดอัตโนมัติ)
  applyMotorSpeeds(speed_motorL, speed_motorR);

  // If distance is 0 or not specified, run one single animation cycle (default mode)
  if (distance_cm <= 0) {
    int total_frames = sizeof(walk) / sizeof(walk[0]);
    for (int i = 0; i < total_frames; i++) {
      loadFlippedYFrame(walk[i]);
      delay(walk[i][3]);
    }
    return;
  }

  // Running for specified distance (cm)
  long target_pulses = (long)(distance_cm / CM_PER_PULSE);
  int total_frames = sizeof(walk) / sizeof(walk[0]);
  int anim_frame = 0;
  unsigned long last_anim_time = millis();
  unsigned long last_print_time = millis();

  sendTelemetry("[Forward] Fixed Speed - L: " + String(speed_motorL) + " | R: " + String(speed_motorR) + " | Target: " + String(distance_cm, 1) + " cm (" + String(target_pulses) + " pulses)");

  while (true) {
    long current_avg = (pulse_count_L + pulse_count_R) / 2;
    if (current_avg >= target_pulses) break;

    // Fixed speeds: คุมความเร็วนิ่งสนิท ไม่มีการดึงสวิงไปมา
    applyMotorSpeeds(speed_motorL, speed_motorR);

    // LED Matrix Animation
    if (millis() - last_anim_time >= walk[anim_frame][3]) {
      loadFlippedYFrame(walk[anim_frame]);
      anim_frame = (anim_frame + 1) % total_frames;
      last_anim_time = millis();
    }

    processBLE();
    if (emergency_stop) {
      stop();
      break;
    }

    // Telemetry print every 200 ms (แสดงค่าพัลส์ให้เห็นชัดเจนสำหรับปรับแต่งความเร็ว)
    if (millis() - last_print_time >= 200) {
      String t = " L: " + String(pulse_count_L) + " | R: " + String(pulse_count_R) + " | PWM: " + String(speed_motorL) + "/" + String(speed_motorR);
      sendTelemetry(t);
      last_print_time = millis();
    }

    delay(10);
  }

  // Active Brake on arrival
  brake(100);
  stop();
  delay(50); // Settle
}

// Overload: กำหนดความเร็วแยกอิสระ (speed_motorL - B_L, speed_motorR - B_R, distance_cm)
void forward(int speed_motorL, int speed_motorR, int B_L, int B_R, float distance_cm = 0) {
  forward(speed_motorL - B_L, speed_motorR - B_R, distance_cm);
}

// ฟังก์ชันเรียกใช้งานง่าย: ใส่แค่ระยะทาง (cm) ระบบใช้ความเร็ว SPEED_L และ SPEED_R คงที่อัตโนมัติ
void forward(float distance_cm = 0) {
  forward(SPEED_L, SPEED_R, distance_cm);
}

// Backward by distance with direct fixed motor speeds
void backward(int speed_motorL, int speed_motorR, float distance_cm = 0) {
  resetEncoders();
  resetYaw();

  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);

  applyMotorSpeeds(speed_motorL, speed_motorR);

  if (distance_cm <= 0) {
    int total_frames = sizeof(walk) / sizeof(walk[0]);
    for (int i = 0; i < total_frames; i++) {
      loadFlippedXFrame(walk[i]);
      delay(walk[i][3]);
    }
    return;
  }

  long target_pulses = (long)(distance_cm / CM_PER_PULSE);
  while ((pulse_count_L + pulse_count_R) / 2 < target_pulses) {
    applyMotorSpeeds(speed_motorL, speed_motorR);
    delay(10);
  }
  brake(100);
  stop();
  delay(50);
}

void backward(int speed_motorL, int speed_motorR, int B_L, int B_R, float distance_cm = 0) {
  backward(speed_motorL - B_L, speed_motorR - B_R, distance_cm);
}

void backward(float distance_cm = 0) {
  backward(SPEED_L, SPEED_R, distance_cm);
}

// Pivot Turn Right by exact degrees (e.g. 90.0)
void turn_right(float target_deg = 90.0, int speed = -1) {
  if (speed <= 0) speed = TURN_SPEED;
  resetEncoders();
  resetYaw();

  // หมุนขวา (ตามเข็ม): ล้อซ้ายเดินหน้า (IN3=0, IN4=1), ล้อขวาถอยหลัง (IN1=1, IN2=0)
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);

  applyMotorSpeeds(speed, speed); // จ่ายไฟเต็มพิกัดสูงสุด 65535 ทั้งสองล้อ ไม่กระตุก

  if (has_mpu) {
    // Gyroscope mode: wait until yaw angle reaches target
    while (true) {
      updateYaw();
      if (abs(current_yaw) >= target_deg) break;
      delay(2);
    }
  } else {
    // Encoder fallback: calculate arc distance per wheel (คูณ TURN_DEG_SCALE ชดเชยแรงหนืดล้ออิสระที่ล็อกไว้)
    long target_pulses = (long)(((target_deg * TURN_DEG_SCALE) / 360.0) * (3.14159265 * TRACK_WIDTH_CM / CM_PER_PULSE));
    while ((pulse_count_L + pulse_count_R) / 2 < target_pulses) {
      delay(2);
    }
  }

  // --- ACTIVE BRAKE & LOCK WHEELS ---
  // 1. สวนกระแสมอเตอร์สั้นๆ (Counter-torque) เพื่อหยุดแรงเฉื่อยการหมุนทันที ไม่ให้แฉลบเลยจุด
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  applyMotorSpeeds(speed, speed);
  delay(BRAKE_REVERSE_MS);

  // 2. ล็อกล้อด้วยระบบ Dynamic Brake (ชอร์ตขั้วมอเตอร์ตรึงล้อให้อยู่นิ่งสนิท)
  brake(BRAKE_HOLD_MS);
  stop();
  sendTelemetry("[Turn Right] Target: " + String(target_deg, 1) + " deg | Pulses: " + String((pulse_count_L + pulse_count_R) / 2));
  delay(100); // Settle pause
}

// Pivot Turn Left by exact degrees (e.g. 90.0)
void turn_left(float target_deg = 90.0, int speed = -1) {
  if (speed <= 0) speed = TURN_SPEED;
  resetEncoders();
  resetYaw();

  // หมุนซ้าย (ทวนเข็ม): ล้อซ้ายถอยหลัง (IN3=1, IN4=0), ล้อขวาเดินหน้า (IN1=0, IN2=1)
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);

  applyMotorSpeeds(speed, speed); // จ่ายไฟเต็มพิกัดสูงสุด 65535 ทั้งสองล้อ ไม่กระตุก

  if (has_mpu) {
    while (true) {
      updateYaw();
      if (abs(current_yaw) >= target_deg) break;
      delay(2);
    }
  } else {
    // Encoder fallback: calculate arc distance per wheel (คูณ TURN_DEG_SCALE ชดเชยแรงหนืดล้ออิสระที่ล็อกไว้)
    long target_pulses = (long)(((target_deg * TURN_DEG_SCALE) / 360.0) * (3.14159265 * TRACK_WIDTH_CM / CM_PER_PULSE));
    while ((pulse_count_L + pulse_count_R) / 2 < target_pulses) {
      delay(2);
    }
  }

  // --- ACTIVE BRAKE & LOCK WHEELS ---
  // 1. สวนกระแสมอเตอร์สั้นๆ (Counter-torque) เพื่อหยุดแรงเฉื่อยการหมุนทันที
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  applyMotorSpeeds(speed, speed);
  delay(BRAKE_REVERSE_MS);

  // 2. ล็อกล้อด้วยระบบ Dynamic Brake
  brake(BRAKE_HOLD_MS);
  stop();
  sendTelemetry("[Turn Left] Target: " + String(target_deg, 1) + " deg | Pulses: " + String((pulse_count_L + pulse_count_R) / 2));
  delay(100);
}

void Sv(int degree) {
  servo.write(degree);
  delay(500);
}

// ============================================================================
// 7. LED MATRIX DISPLAY TRANSFORM HELPERS
// ============================================================================
void loadTransformedFrame(const uint32_t frame[4], bool flipX, bool flipY) {
  uint32_t transformed[4] = {0, 0, 0, frame[3]};
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 12; x++) {
      int srcBit = y * 12 + x;
      int srcWord = srcBit / 32;
      int srcPos = 31 - (srcBit % 32);
      bool isSet = (frame[srcWord] >> srcPos) & 1;
      if (isSet) {
        int new_x = flipX ? (11 - x) : x;
        int new_y = flipY ? (7 - y) : y;
        int dstBit = new_y * 12 + new_x;
        int dstWord = dstBit / 32;
        int dstPos = 31 - (dstBit % 32);
        transformed[dstWord] |= (1UL << dstPos);
      }
    }
  }
  matrix.loadFrame(transformed);
}

void loadFlippedXFrame(const uint32_t frame[4]) {
  loadTransformedFrame(frame, true, false);
}

void loadFlippedYFrame(const uint32_t frame[4]) {
  loadTransformedFrame(frame, false, true);
}

void loadFlippedXYFrame(const uint32_t frame[4]) {
  loadTransformedFrame(frame, true, true);
}

// ============================================================================
// 8. SETUP
// ============================================================================
void setup() {
  Serial.begin(9600);
  delay(1000); // 1-second delay for USB Serial enumeration & connection

  Serial.println();
  Serial.println("=========================================");
  Serial.println("   IOT ROBOT SYSTEM ONLINE (COM PORT OK) ");
  Serial.println("=========================================");

  matrix.begin();

  // Motor pins & 16-bit PWM Resolution (0 - 65535)
  analogWriteResolution(16);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stop();

  // Servo
  servo.attach(SV);

  // Encoders
  pinMode(ENCODER_L, INPUT_PULLUP);
  pinMode(ENCODER_R, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_L), isr_count_L, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_R), isr_count_R, RISING);

  // Initialize MPU-6050
  initMPU6050();

  // Initialize Bluetooth Low Energy (BLE)
  if (USE_BLE) {
    if (BLE.begin()) {
      BLE.setLocalName("IOT-ROBOT");
      BLE.setDeviceName("IOT-ROBOT");
      BLE.setAdvertisedService(uartService);
      uartService.addCharacteristic(txChar);
      uartService.addCharacteristic(rxChar);
      BLE.addService(uartService);
      BLE.advertise();
      Serial.println("[BLE] Bluetooth Online! Device Name: IOT-ROBOT");
    } else {
      Serial.println("[BLE] Warning: BLE failed to initialize!");
    }
  }

  Serial.println("System Ready! Waiting to start...");
}

// ============================================================================
// 9. LOOP - SEQUENCE RUNNER
// ============================================================================
bool mission_completed = false;

void loop() {
  if (!mission_completed) {
    Serial.println("Place robot at Start Line. Race starting in 3 seconds...");
    delay(500); // 1-second delay for positioning
    
    // ========================================================================
    // ตัวอย่างการวิ่งกรอบสี่เหลี่ยม 4 จุด (450 cm x 540 cm วนขวา)
    // คุณสามารถปรับแต่งขั้นตอนตามที่ต้องการได้เลยครับ:
    // ========================================================================
    
    
    // ------------------------------------------------------------------------
    // Leg 1: วิ่งตรง 350 cm (ความเร็วซ้าย 54000-16000=38000, ขวา 54000-0=54000, ระยะ 350 cm)
    // ------------------------------------------------------------------------
    Serial.println(">>> START: Forward Leg 1 (350 cm) <<<");
    forward(54000, 54000, 0, 3000, 350);
    turn_right(555.0);
    // forward(54000, 54000, 16000, 0, 410); // Leg 2 (ตัวอย่าง)
    

      
    /*
    
    // จุดที่ 1: วิ่งตรง 450 cm
    Serial.println("Leg 1: Forward 450 cm");
    forward(200, 200, 0, 0, 350.0);

    // เลี้ยวขวา 90 องศา
    Serial.println("Turn Right 90 deg");
    turn_right(420.0);

    // จุดที่ 2: วิ่งตรง 540 cm
    Serial.println("Leg 2: Forward 540 cm");
    forward(200, 200, 0, 0, 420.0);

    // เลี้ยวขวา 90 องศา
    Serial.println("Turn Right 90 deg");
    turn_right(420.0);

    // จุดที่ 3: วิ่งตรง 450 cm
    Serial.println("Leg 3: Forward 450 cm");
    forward(200, 200, 0, 0, 390.0);

    // เลี้ยวขวา 90 องศา
    Serial.println("Turn Right 90 deg");
    turn_right(420.0);

    // จุดที่ 4: วิ่งตรง 540 cm กลับจุดเริ่มต้น
    Serial.println("Leg 4: Forward 540 cm");
    forward(200, 200, 0, 0, 420.0);

    // เลี้ยวขวา 90 องศา สู่ทิศเริ่มต้น
    Serial.println("Turn Right 90 deg (Finish)");
    turn_right(420.0);
    */
    
    stop();
    mission_completed = true;
    Serial.println("=== Course Completed! Press RESET to run again. ===");
  }
}
