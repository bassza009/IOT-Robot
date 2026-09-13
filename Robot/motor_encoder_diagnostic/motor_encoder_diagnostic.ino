// ============================================================================
// MOTOR & ENCODER DIAGNOSTIC TOOL (Arduino UNO R4 WiFi)
// เครื่องมือทดสอบมอเตอร์และเอ็นโค้ดเดอร์แยกซ้าย-ขวา แบบละเอียด 100%
// ============================================================================

#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

// ----------------------------------------------------------------------------
// PIN DEFINITIONS
// ----------------------------------------------------------------------------
// Motor Driver Channel A (Pin 10, 9, 8)
const int ENA = 10;
const int IN1 = 9;
const int IN2 = 8;

// Motor Driver Channel B (Pin 5, 7, 6)
const int ENB = 5;
const int IN3 = 7;
const int IN4 = 6;

// LM393 Speed Optical Encoders
const int ENCODER_L = 3;  // ขา 3
const int ENCODER_R = 11; // ขา 11

// ----------------------------------------------------------------------------
// ENCODER COUNTERS (Volatile for ISR)
// ----------------------------------------------------------------------------
volatile unsigned long count_L = 0;
volatile unsigned long count_R = 0;

void countPulseL() { count_L++; }
void countPulseR() { count_R++; }

// Test Speed: 16-bit PWM (0 - 65535)
const int TEST_PWM = 55000; 

// ----------------------------------------------------------------------------
// MOTOR CONTROL FUNCTIONS
// ----------------------------------------------------------------------------
void stopAll() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 0);
}

void brakeAll(int hold_ms = 100) {
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 65535);
  analogWrite(ENB, 65535);
  delay(hold_ms);
  stopAll();
}

// ----------------------------------------------------------------------------
// INDIVIDUAL MOTOR TESTS
// ----------------------------------------------------------------------------
void testMotorA() {
  Serial.println();
  Serial.println("==================================================");
  Serial.println(">>> [TEST 1] หมุนเฉพาะ Channel A (ENA: ขา 10, IN1: 9, IN2: 8) <<<");
  Serial.println("==================================================");
  Serial.println("-> สังเกตตัวรถ: ล้อฝั่งไหนหมุน? (ซ้าย หรือ ขวา?)");
  Serial.println("-> ความเร็วทดสอบ: 55,000 PWM (16-bit) เป็นเวลา 3 วินาที");
  Serial.println("เริ่มหมุนใน 1 วินาที...");
  delay(1000);

  count_L = 0;
  count_R = 0;

  // เดินหน้าเฉพาะ Channel A
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  analogWrite(ENA, TEST_PWM);

  // ปิด Channel B สนิท
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 0);
  analogWrite(ENB, 0);

  unsigned long start_t = millis();
  while (millis() - start_t < 3000) {
    Serial.print("กำลังหมุน... พัลส์ Pin 3: ");
    Serial.print(count_L);
    Serial.print(" | พัลส์ Pin 11: ");
    Serial.println(count_R);
    delay(250);
  }

  brakeAll(150);
  Serial.println();
  Serial.println("--- สรุปผล Channel A (ขา 10) ---");
  Serial.print("พัลส์รวม Pin 3:  "); Serial.println(count_L);
  Serial.print("พัลส์รวม Pin 11: "); Serial.println(count_R);
  if (count_L > count_R) {
    Serial.println("=> ผลลัพธ์: Channel A เชื่อมโยงกับ Encoder Pin 3");
  } else if (count_R > count_L) {
    Serial.println("=> ผลลัพธ์: Channel A เชื่อมโยงกับ Encoder Pin 11");
  } else {
    Serial.println("=> ข้อควรระวัง: ไม่พบสัญญาณพัลส์ ตรวจสอบการต่อสาย Encoder");
  }
  Serial.println("==================================================");
  Serial.println();
}

void testMotorB() {
  Serial.println();
  Serial.println("==================================================");
  Serial.println(">>> [TEST 2] หมุนเฉพาะ Channel B (ENB: ขา 5, IN3: 7, IN4: 6) <<<");
  Serial.println("==================================================");
  Serial.println("-> สังเกตตัวรถ: ล้อฝั่งไหนหมุน? (ซ้าย หรือ ขวา?)");
  Serial.println("-> ความเร็วทดสอบ: 55,000 PWM (16-bit) เป็นเวลา 3 วินาที");
  Serial.println("เริ่มหมุนใน 1 วินาที...");
  delay(1000);

  count_L = 0;
  count_R = 0;

  // ปิด Channel A สนิท
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  analogWrite(ENA, 0);

  // เดินหน้าเฉพาะ Channel B
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENB, TEST_PWM);

  unsigned long start_t = millis();
  while (millis() - start_t < 3000) {
    Serial.print("กำลังหมุน... พัลส์ Pin 3: ");
    Serial.print(count_L);
    Serial.print(" | พัลส์ Pin 11: ");
    Serial.println(count_R);
    delay(250);
  }

  brakeAll(150);
  Serial.println();
  Serial.println("--- สรุปผล Channel B (ขา 5) ---");
  Serial.print("พัลส์รวม Pin 3:  "); Serial.println(count_L);
  Serial.print("พัลส์รวม Pin 11: "); Serial.println(count_R);
  if (count_L > count_R) {
    Serial.println("=> ผลลัพธ์: Channel B เชื่อมโยงกับ Encoder Pin 3");
  } else if (count_R > count_L) {
    Serial.println("=> ผลลัพธ์: Channel B เชื่อมโยงกับ Encoder Pin 11");
  } else {
    Serial.println("=> ข้อควรระวัง: ไม่พบสัญญาณพัลส์ ตรวจสอบการต่อสาย Encoder");
  }
  Serial.println("==================================================");
  Serial.println();
}

void testBothMotors() {
  Serial.println();
  Serial.println("==================================================");
  Serial.println(">>> [TEST 3] หมุน 2 ล้อพร้อมกันที่ PWM เท่ากันเป๊ะ (55,000) <<<");
  Serial.println("==================================================");
  Serial.println("-> วัตถุประสงค์: วัดความเร็วตามธรรมชาติจริงของมอเตอร์ 2 ข้าง");
  Serial.println("เริ่มหมุนใน 1 วินาที...");
  delay(1000);

  count_L = 0;
  count_R = 0;

  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, TEST_PWM);
  analogWrite(ENB, TEST_PWM);

  unsigned long start_t = millis();
  while (millis() - start_t < 3000) {
    Serial.print("กำลังวิ่ง... Pin 3: ");
    Serial.print(count_L);
    Serial.print(" | Pin 11: ");
    Serial.print(count_R);
    Serial.print(" | ส่วนต่าง (3 - 11): ");
    Serial.println((long)count_L - (long)count_R);
    delay(250);
  }

  brakeAll(150);
  Serial.println();
  Serial.println("--- สรุปผลการทดสอบความเร็วตามธรรมชาติ (Benchmark) ---");
  Serial.print("พัลส์รวม Pin 3:  "); Serial.println(count_L);
  Serial.print("พัลส์รวม Pin 11: "); Serial.println(count_R);
  
  if (count_L > 0 && count_R > 0) {
    float ratio = (float)count_L / (float)count_R;
    Serial.print("อัตราส่วนความเร็ว (Pin 3 / Pin 11): "); Serial.println(ratio, 4);
    if (count_L > count_R) {
      float diff_pct = ((float)(count_L - count_R) / (float)count_R) * 100.0;
      Serial.print("=> Pin 3 หมุนเร็วกว่า Pin 11 อยู่: "); Serial.print(diff_pct, 2); Serial.println(" %");
    } else if (count_R > count_L) {
      float diff_pct = ((float)(count_R - count_L) / (float)count_L) * 100.0;
      Serial.print("=> Pin 11 หมุนเร็วกว่า Pin 3 อยู่: "); Serial.print(diff_pct, 2); Serial.println(" %");
    } else {
      Serial.println("=> มอเตอร์ทั้ง 2 ข้างหมุนเร็วเท่ากันเป๊ะ 100%!");
    }
  }
  Serial.println("==================================================");
  Serial.println();
}

void printMenu() {
  Serial.println();
  Serial.println("==================================================");
  Serial.println("      เมนูทดสอบมอเตอร์และเอ็นโค้ดเดอร์ (DIAGNOSTIC)");
  Serial.println("==================================================");
  Serial.println("  กด '1' หรือ 'A' : ทดสอบเฉพาะ Channel A (ขา 10, 9, 8)");
  Serial.println("  กด '2' หรือ 'B' : ทดสอบเฉพาะ Channel B (ขา 5, 7, 6)");
  Serial.println("  กด '3' หรือ 'T' : ทดสอบ 2 ล้อพร้อมกัน (วัดค่าสมดุลจริง)");
  Serial.println("  กด '4' หรือ 'X' : รันการทดสอบทั้งหมดอัตโนมัติ (Auto Test)");
  Serial.println("==================================================");
  Serial.print("เลือกคำสั่ง: ");
}

// ----------------------------------------------------------------------------
// SETUP & LOOP
// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // ตั้งค่า PWM เป็น 16-bit (0 - 65535)
  analogWriteResolution(16);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENCODER_L, INPUT_PULLUP);
  pinMode(ENCODER_R, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_L), countPulseL, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_R), countPulseR, FALLING);

  matrix.begin();
  stopAll();

  Serial.println("\n[SYSTEM] Diagnostic Ready! 16-bit PWM Initialized.");
  printMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '\r' || cmd == '\n' || cmd == ' ') return;

    Serial.println(cmd);

    switch (cmd) {
      case '1':
      case 'a':
      case 'A':
        testMotorA();
        printMenu();
        break;

      case '2':
      case 'b':
      case 'B':
        testMotorB();
        printMenu();
        break;

      case '3':
      case 't':
      case 'T':
        testBothMotors();
        printMenu();
        break;

      case '4':
      case 'x':
      case 'X':
        Serial.println("\n>>> รันการทดสอบอัตโนมัติทั้ง 3 ขั้นตอน <<<");
        testMotorA();
        delay(1500);
        testMotorB();
        delay(1500);
        testBothMotors();
        printMenu();
        break;

      default:
        Serial.println("คำสั่งไม่ถูกต้อง! กรุณาเลือก 1, 2, 3 หรือ 4");
        printMenu();
        break;
    }
  }
}
