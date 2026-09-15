#pragma once

// FULL-SPEED PRESET. Supply reported by user: 6.8 V (set externally).
// PWM controls motor power; this sketch does not measure or regulate voltage.
// EDIT HERE FIRST. Sensor order: left -> right, A5 A4 A3 A2 A1.
namespace settings {
const int FLOOR_RAW[5] = {3000, 3000, 3000, 3000, 3000};
const int BLACK_RAW[5] = {900, 900, 900, 900, 900};
const int LINE_THRESHOLD = 450; // Normalized darkness: floor=0, black=1000.

// PWM is 0..255 in this sketch (the old sketch used 0..65535).
const int BASE_SPEED = 255;     // เดินตรงเต็ม PWM; ล้อขวายังชดเชยด้วย RIGHT_TRIM
constexpr float STEERING_KP = 128; // เพิ่มตามสัดส่วนความเร็ว 80 * 255 / 160 ≈ 128
const int TURN_SPEED = 255;     // หมุนเข้ามุม 90° / มุมแหลมเต็ม PWM ก่อนชดเชยล้อขวา
const int SEARCH_SPEED = 180;   // หมุนต่อหาเส้นไวขึ้นเมื่อเส้นหายระหว่างเข้าโค้ง
const int REVERSE_SPEED = 100;  // ใช้กำลังถอยจากรุ่นที่ทดสอบวิ่งได้แล้ว
const int CROSS_SPEED = 110;    // Forward speed while all five see black.
const int MAX_SPEED = 255;      // PWM สูงสุดของบอร์ด 8 บิต
constexpr float RIGHT_TRIM = 46800.0f / 55500.0f; // Original wheel balance.

const unsigned long SEARCH_MS = 600;        // Lost longer than this -> reverse recovery.
const unsigned long TURN_TIMEOUT_MS = 1800; // Stuck in a sharp turn -> reverse recovery.
const unsigned long REVERSE_MS = 650;      // ถอยครบเวลาแล้วรอเส้น เจอเส้นกลับวิ่งเอง
const unsigned long BRAKE_MS = 80;         // เบรกก่อนถอยและก่อนกลับเดินหน้า
const unsigned long ALL_BLACK_MS = 250;    // ดำครบห้าจุดนานเกินนี้ -> ถอยหาเส้น
const unsigned long SAMPLE_MS = 5;         // Control interval; no delay().
const unsigned long REPORT_MS = 100;       // Serial sensor report interval.
const unsigned long CONTROL_GAP_MS = 150;  // Late control update -> STOP.
}
