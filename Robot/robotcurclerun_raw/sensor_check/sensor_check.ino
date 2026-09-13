/*
 * ============================================================================
 * สเก็ตช์สำหรับตรวจเช็คและคาลิเบรตเซนเซอร์เส้น 5 จุด (TCRT5000)
 * บอร์ด: Arduino UNO R4 WiFi
 * ============================================================================
 * ลำดับเซนเซอร์ (มองจากท้ายรถไปหน้ารถ):
 *   A5: ซ้ายสุด (Left Outer / L2)
 *   A4: ซ้ายใน  (Left Inner / L1)
 *   A3: กลาง    (Center / C)
 *   A2: ขวาใน  (Right Inner / R1)
 *   A1: ขวาสุด (Right Outer / R2)
 *
 * การต่อสายมอเตอร์: ปิดการทำงานมอเตอร์ทั้งหมดเพื่อความปลอดภัยขณะทดสอบ
 *
 * คำสั่งผ่าน Serial Monitor (ตั้งค่า Baud Rate ที่ 115200):
 *   - พิมพ์ 'r' แล้วกด Enter: รีเซ็ตค่า Min/Max เริ่มบันทึกใหม่
 *   - พิมพ์ 'c' แล้วกด Enter: สรุปโค้ด C++ นำไปวางใน LineFollower.h ได้ทันที
 *   - พิมพ์ 'p' แล้วกด Enter: สลับโหมดการแสดงผล (แบบกระชับ A5:... A4:... / แบบละเอียด)
 * ============================================================================
 */

const int SENSOR_PINS[5] = {A5, A4, A3, A2, A1};
const char* PIN_LABELS[5] = {"A5 (ซ้ายสุด)", "A4 (ซ้ายใน)", "A3 (กลาง)", "A2 (ขวาใน)", "A1 (ขวาสุด)"};
const char* SHORT_NAMES[5] = {"A5", "A4", "A3", "A2", "A1"};

// ขามอเตอร์เดิม (ปิดการทำงานทั้งหมดเพื่อความปลอดภัย)
const int ENA = 5;
const int IN1 = 6;
const int IN2 = 7;
const int ENB = 10;
const int IN3 = 9;
const int IN4 = 8;

int rawValues[5] = {0};
int minValues[5] = {4095, 4095, 4095, 4095, 4095};
int maxValues[5] = {0, 0, 0, 0, 0};

bool compactMode = false;
uint32_t lastPrintMs = 0;
const uint32_t PRINT_INTERVAL_MS = 200; // อัปเดตการแสดงผลทุก 200 ms

// อ่านค่าแบบเดียวกับโค้ดจริง: ทิ้งค่าแรกหลังสลับช่อง ADC + กรองค่ามัธยฐาน 3 ตัว (Median-3)
int readSensor(int pin) {
  analogRead(pin); // สลับช่อง ADC แล้วทิ้งค่าแรกเพื่อให้ sample-and-hold นิ่ง
  int a = analogRead(pin);
  int b = analogRead(pin);
  int c = analogRead(pin);
  if (a > b) { int t = a; a = b; b = t; }
  if (b > c) { int t = b; b = c; c = t; }
  if (a > b) b = a;
  return b;
}

void resetMinMax() {
  for (int i = 0; i < 5; ++i) {
    minValues[i] = 4095;
    maxValues[i] = 0;
  }
  Serial.println("\n[RESET] รีเซ็ตค่า Min/Max เรียบร้อยแล้ว! นำหุ่นไปเลื่อนผ่านพื้นไม้และเส้นดำเพื่อเก็บค่าใหม่");
}

void printGeneratedConfig() {
  Serial.println("\n========================================================");
  Serial.println("  >>> โค้ดสำหรับคัดลอกไปวางใน LineFollower.h <<<");
  Serial.println("========================================================");
  
  // ตรวจสอบว่าดำให้ค่าสูงกว่าหรือต่ำกว่าไม้ (โดยทั่วไป TCRT5000: ไม้ ~3700 สูง, ดำ ~600 ต่ำ)
  Serial.println("// ค่าที่อ่านได้จริงจากตัวหุ่น (เรียง A5, A4, A3, A2, A1):");
  
  // สมมติฐานพื้นฐาน: ค่า Max คือพื้นไม้ (สะท้อนดี), Min คือเส้นดำ (สะท้อนน้อย)
  // หากโมดูลของคุณกลับขั้ว สลับตัวแปรตามความจริง
  Serial.print("  int woodRaw[SENSOR_COUNT]  = {");
  for (int i = 0; i < 5; ++i) {
    Serial.print(maxValues[i]);
    if (i < 4) Serial.print(", ");
  }
  Serial.println("}; // ค่าพื้นไม้ (สะท้อนสูง)");

  Serial.print("  int blackRaw[SENSOR_COUNT] = {");
  for (int i = 0; i < 5; ++i) {
    Serial.print(minValues[i]);
    if (i < 4) Serial.print(", ");
  }
  Serial.println("}; // ค่าเส้นดำ (สะท้อนต่ำ)");

  // คำนวณค่ากึ่งกลางที่แนะนำ (Threshold 45%)
  Serial.println("  int lineThreshold = 450; // เกณฑ์ 45% ระหว่างไม้กับดำ");
  Serial.println("========================================================\n");
}

void setup() {
  // 1. ปิดมอเตอร์ทั้งหมดเพื่อความปลอดภัย
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // ไฟติดแสดงสถานะพร้อมทำงาน

  // 2. ตั้งค่า ADC 12 บิต (0-4095) บน UNO R4 WiFi
  analogReadResolution(12);
  for (int i = 0; i < 5; ++i) {
    pinMode(SENSOR_PINS[i], INPUT);
  }

  // 3. เริ่มต้น Serial
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================================");
  Serial.println("  ระบบตรวจสอบเซนเซอร์ตามเส้น TCRT5000 (UNO R4 WiFi)");
  Serial.println("========================================================");
  Serial.println("คำแนะนำ:");
  Serial.println("  1. เลื่อนเซนเซอร์แต่ละตัวผ่าน 'พื้นไม้' และ 'เส้นดำ'");
  Serial.println("  2. โปรแกรมจะจดจำค่า Min และ Max ให้โดยอัตโนมัติ");
  Serial.println("  3. พิมพ์ 'c' แล้ว Enter เพื่อสร้างโค้ดสำหรับ LineFollower.h");
  Serial.println("  4. พิมพ์ 'r' แล้ว Enter เพื่อเริ่มเก็บค่าใหม่");
  Serial.println("  5. พิมพ์ 'p' แล้ว Enter เพื่อสลับโหมดข้อความสั้น / โหมดตาราง");
  Serial.println("========================================================\n");
}

void loop() {
  // ตรวจรับคำสั่งจากผู้ใช้
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'r' || ch == 'R') {
      resetMinMax();
    } else if (ch == 'c' || ch == 'C') {
      printGeneratedConfig();
    } else if (ch == 'p' || ch == 'P') {
      compactMode = !compactMode;
      Serial.println(compactMode ? "[MODE] สลับเป็นโหมดบรรทัดเดียว (Compact)" : "[MODE] สลับเป็นโหมดตาราง (Table)");
    }
  }

  // อ่านค่าเซนเซอร์ทั้ง 5 ขา
  for (int i = 0; i < 5; ++i) {
    int val = readSensor(SENSOR_PINS[i]);
    rawValues[i] = val;
    if (val < minValues[i]) minValues[i] = val;
    if (val > maxValues[i]) maxValues[i] = val;
  }

  // แสดงผลตามช่วงเวลา
  uint32_t now = millis();
  if (now - lastPrintMs >= PRINT_INTERVAL_MS) {
    lastPrintMs = now;

    if (compactMode) {
      // แสดงผลแบบบรรทัดเดียวกระชับตามที่ผู้ใช้ต้องการ: A5:... A4:... A3:... A2:... A1:...
      char lineBuf[128];
      snprintf(lineBuf, sizeof(lineBuf),
        "A5:%-4d  A4:%-4d  A3:%-4d  A2:%-4d  A1:%-4d",
        rawValues[0], rawValues[1], rawValues[2], rawValues[3], rawValues[4]);
      Serial.println(lineBuf);
    } else {
      // แสดงผลแบบละเอียดพร้อมกราฟิกและค่า Min/Max
      Serial.println("----------------------------------------------------------------");
      for (int i = 0; i < 5; ++i) {
        // คำนวณเปอร์เซ็นต์ความดำตามช่วงที่เคยวัดได้
        int span = maxValues[i] - minValues[i];
        int percent = 0;
        if (span > 50) {
          // หากสะท้อนน้อย = ดำ (ค่าต่ำกว่า)
          percent = constrain((maxValues[i] - rawValues[i]) * 100 / span, 0, 100);
        }

        // กราฟแท่งแสดงความดำ 10 ขีด
        int bars = percent / 10;
        char barStr[13] = "[          ]";
        for (int b = 0; b < bars && b < 10; ++b) {
          barStr[b + 1] = '=';
        }

        char outBuf[128];
        snprintf(outBuf, sizeof(outBuf),
          "%-14s | RAW: %4d | %s (%3d%%) | Min: %4d | Max: %4d",
          PIN_LABELS[i], rawValues[i], barStr, percent, minValues[i], maxValues[i]);
        Serial.println(outBuf);
      }
      // แสดงบรรทัดสรุปแบบระบุขาชัดเจน
      char summaryBuf[128];
      snprintf(summaryBuf, sizeof(summaryBuf),
        ">> ปัจจุบัน:  A5:%-4d  A4:%-4d  A3:%-4d  A2:%-4d  A1:%-4d",
        rawValues[0], rawValues[1], rawValues[2], rawValues[3], rawValues[4]);
      Serial.println(summaryBuf);
    }
  }
}
