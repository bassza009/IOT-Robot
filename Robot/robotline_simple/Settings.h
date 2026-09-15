#pragma once

// EDIT HERE FIRST. Sensor order: left -> right, A5 A4 A3 A2 A1.
namespace settings {
const int FLOOR_RAW[5] = {3000, 3000, 3000, 3000, 3000};
const int BLACK_RAW[5] = {900, 900, 900, 900, 900};
const int LINE_THRESHOLD = 450; // Normalized darkness: floor=0, black=1000.

// PWM is 0..255 in this sketch (the old sketch used 0..65535).
const int BASE_SPEED = 160;     // Normal forward speed. Tune this first.
constexpr float STEERING_KP = 80; // แก้โค้งธรรมดาแรงขึ้น; ลดทีละ 5 ถ้าส่าย
const int TURN_SPEED = 210;     // หมุนสองล้อสวนกันเร็วขึ้นสำหรับมุม 90° / มุมแหลม
const int SEARCH_SPEED = 160;   // หมุนต่อหาเส้นไวขึ้นเมื่อเส้นหายระหว่างเข้าโค้ง
const int REVERSE_SPEED = 160;  // กำลังถอยเท่ากำลังเดินตั้งต้น ช่วยออกตัวหลังเบรก
const int CROSS_SPEED = 110;    // Forward speed while all five see black.
const int MAX_SPEED = 240;      // เผื่อกำลังล้อนอกขณะเลี้ยว; PWM สูงสุดของบอร์ด 255
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
