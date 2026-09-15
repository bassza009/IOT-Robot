#pragma once

// HYPER-DASH 3: Tamiya specifies 2.4..3.0 V, 1.6..3.0 A at recommended load.
// These describe the actual hardware; changing numbers does NOT change its power.
// Run override requested for L298N + 6.8 V. This does not make the hardware suitable.
namespace settings {
const bool ALLOW_UNVERIFIED_MOTOR_POWER = true; // false = restore power-rating lock.
const int MOTOR_SUPPLY_MV = 6800; // Actual motor power supply, not Arduino logic supply.
const int DRIVER_DC_LIMIT_MA = 2000; // L298 IC DC absolute maximum per channel.

// Higher-duty trial after PWM 100 did not start the user's motor.
// These new values have not been validated on the physical motor/driver.
// No current/voltage feedback or RPM regulation is implemented.
// Sensor order: left -> right, A5 A4 A3 A2 A1.
const int FLOOR_RAW[5] = {3000, 3000, 3000, 3000, 3000};
const int BLACK_RAW[5] = {900, 900, 900, 900, 900};
const int LINE_THRESHOLD = 450; // Normalized darkness: floor=0, black=1000.

// PWM is 0..255 in this sketch (the old sketch used 0..65535).
const int BASE_SPEED = 160;     // ความเร็วทางตรง เพิ่มจาก 100 ที่มอเตอร์ยังไม่เริ่มหมุน
constexpr float STEERING_KP = 80; // แรงแก้โค้ง; ลดทีละ 5 ถ้าส่าย
const int TURN_SPEED = 180;     // กำลังหมุนเข้ามุม
const int SEARCH_SPEED = 160;   // หมุนหาเส้น
const int REVERSE_SPEED = 160;  // ถอยหาเส้นเมื่อ error
const int CROSS_SPEED = 140;    // ผ่านดำครบห้าจุดช่วงสั้น
const int MAX_SPEED = 180;      // เพดานขณะขับ ไม่ใช่ตัวจำกัดแรงดัน/กระแส; เบรกยังใช้ enable 255
constexpr float RIGHT_TRIM = 1.0f; // มอเตอร์คู่ใหม่: เริ่มเท่ากัน แล้วจูนสมดุลใหม่

const unsigned long SEARCH_MS = 600;        // Lost longer than this -> reverse recovery.
const unsigned long TURN_TIMEOUT_MS = 1800; // Stuck in a sharp turn -> reverse recovery.
const unsigned long REVERSE_MS = 650;      // ถอยครบเวลาแล้วรอเส้น เจอเส้นกลับวิ่งเอง
const unsigned long BRAKE_MS = 80;         // เบรกก่อนถอยและก่อนกลับเดินหน้า
const unsigned long ALL_BLACK_MS = 250;    // ดำครบห้าจุดนานเกินนี้ -> ถอยหาเส้น
const unsigned long SAMPLE_MS = 5;         // Control interval; no delay().
const unsigned long REPORT_MS = 100;       // Serial sensor report interval.
const unsigned long CONTROL_GAP_MS = 150;  // Late control update -> STOP.
}
