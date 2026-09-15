# Simple line follower — UNO R4 WiFi

A separate, smaller version of `robotcurclerun_raw`: one sketch and one settings
file. It uses proportional steering, Serial STOP, timed line search and reverse
recovery on tracking errors. No extra
libraries, BLE, encoder interrupts or reverse-distance calculations are needed.
The original folder and its local tuning changes are preserved.

## Start here / เริ่มใช้งาน

1. Open `robotline_simple.ino` in Arduino IDE. Select **Arduino UNO R4 WiFi**.
2. Edit the **Settings.h** tab, then upload. Motors start on the first sensor
   sample after setup; there is no countdown and no START command.
3. Use Serial Monitor at **115200 baud**. Send **STOP** to brake and stay stopped.
   Lowercase `stop` also works, with or without a newline.
4. After an explicit STOP or a system fault, put the robot back on the line and
   press **RESET** to run again. Tracking errors recover automatically: if the
   reverse attempt times out, `WAIT_LINE` resumes when a normal line is found.

ยกล้อขณะตรวจทิศมอเตอร์และ STOP ครั้งแรก จากนั้นทดสอบกับเส้นจริงที่ความเร็วต่ำ
แก้ค่าจูนใน `Settings.h` ส่วนพฤติกรรมการเลี้ยวแก้ใน `followLine()` ของไฟล์ `.ino`

ดูไฟ **L บนบอร์ด** ได้โดยไม่ต้องต่อ BLE หรือ Serial:

| ไฟ L | ความหมาย |
| --- | --- |
| ติดค้าง | กำลังตามเส้น / เลี้ยว / หมุนหาเส้น |
| กระพริบเร็ว | กำลังถอยหาเส้น (`REVERSE`) |
| กระพริบหนึ่งครั้งต่อวินาที | ถอยครบเวลาแล้ว รอเส้น (`WAIT_LINE`); วางกลับบนเส้นปกติแล้ววิ่งต่อเอง |
| กระพริบเป็นคู่ทุก 1.5 วินาที | `STOP` หรือข้อผิดพลาดระบบที่ต้อง RESET |
| ดับช่วงสั้น ๆ | เบรกก่อนเปลี่ยนทิศ |

## Wiring

Same as the current `robotcurclerun_raw`, viewed in the robot's forward direction:

| Part | Pins |
| --- | --- |
| Sensors, left → right | A5, A4, A3, A2, A1 |
| Left motor | PWM D5, direction D6 / D7 |
| Right motor | PWM D10, direction D9 / D8 |

Forward sets the first direction pin LOW and the second HIGH. Zero speed brakes
with both direction pins HIGH. Existing encoder wires may remain connected;
this sketch does not use them. A4/A5 belong to line sensors, so do not share
these pins with an MPU-6050 or another I2C device.

## Tune Settings.h / ค่าที่ปรับบ่อย

| Setting | Default | Change it when… |
| --- | --- | --- |
| `FLOOR_RAW[5]` | 3000 each | Enter the five measured floor readings. |
| `BLACK_RAW[5]` | 900 each | Enter the five measured black-line readings. |
| `LINE_THRESHOLD` | 450 | A sensor reports black too easily or misses black. |
| `BASE_SPEED` | 160 | Set normal driving speed; increase after following works. |
| `STEERING_KP` | 80 | Increase for stronger curve correction; decrease if weaving. |
| `TURN_SPEED` | 210 | Set the power for sharp left/right pivots, including 90° and acute corners. |
| `RIGHT_TRIM` | 46800 / 55500 ≈ 0.843 | Adjust right-wheel power to balance the motors. |
| `SEARCH_SPEED` / `SEARCH_MS` | 160 / 600 ms | Set the speed and duration of a lost-line search. |
| `TURN_TIMEOUT_MS` | 1800 ms | Begin reverse recovery if a sharp turn never returns to normal tracking. |
| `REVERSE_SPEED` / `REVERSE_MS` | 160 / 650 ms | Set reverse power and the time before stopping to wait for a line. |
| `BRAKE_MS` | 80 ms | Brake before reversing and before returning to forward tracking. |
| `CROSS_SPEED` / `ALL_BLACK_MS` | 110 / 250 ms | Drive briefly across all-black readings, then reverse if they persist. |
| `MAX_SPEED` | 240 | Cap either wheel's commanded PWM. |

**Speeds use 0–255, not the old 0–65535 scale.** An old PWM value divided by 257
gives an approximate equivalent here. These defaults intentionally begin below
the near-maximum speeds in your current file; they are starting points for tuning.
The right-wheel trim is applied before the final speed cap.

จูนให้เลี้ยวไวขึ้น: `STEERING_KP` เพิ่มจาก 55 เป็น 80 สำหรับโค้งธรรมดา,
`TURN_SPEED` เพิ่มจาก 150 เป็น 210 สำหรับการหมุนเข้ามุม และ `SEARCH_SPEED`
เพิ่มจาก 110 เป็น 160 เพื่อหมุนต่อเมื่อเส้นหายระหว่างมุมแหลม
`MAX_SPEED=240` เผื่อกำลังล้อนอกขณะเลี้ยว ส่วนทางตรงใช้ `BASE_SPEED=160`
และถอยแก้ error ใช้ `REVERSE_SPEED=160` เพื่อช่วยออกตัวหลังเบรก
ถ้าส่ายบนโค้งธรรมดาให้ลด `STEERING_KP` ทีละ 5; ถ้าหมุนเลยเส้นที่มุม
ให้ลด `TURN_SPEED` ทีละ 10 หรือ `SEARCH_SPEED` ถ้าเลยเส้นระหว่างสถานะ `SEARCH`
กำลัง PWM ที่เพิ่มขึ้นยังต้องลองกับมุม 90° และมุมสามเหลี่ยมจริง
เซนเซอร์ห้าจุดใช้รูปแบบเส้น ไม่ได้วัดองศามุมโดยตรง

For calibration, send STOP and measure each sensor over the floor and over black
at its normal mounting height. Copy `RAW` values into the two arrays, in physical
left-to-right order. The starting values come from the actual current header;
the old README contains different values. Each channel can have its own readings,
and black can read higher or lower than the floor.

`LINE_THRESHOLD` is normalized: floor = 0, black = 1000. With the defaults,
raw values **≤ 2055** count as black. Increase the threshold to require a darker
reading. Equal endpoints, values outside 0–4095, or an invalid threshold stop
the robot at startup. Invalid motor settings produce a compile error.

## Behavior you can modify

`loop()` services STOP, reads the five sensors every 5 ms, calls `followLine()`,
then sends a short status report. There are no blocking delays. Ordinary curves
use a weighted line position and **P-only** steering, with one steering gain.

| Sensors seeing black | Response |
| --- | --- |
| A3, or balanced A4+A3+A2 | Forward |
| A4 / A4+A3 | Curve left; a center pair makes a gentler correction |
| A2 / A3+A2 | Curve right |
| A5 / A5+A4 / A5+A4+A3 / A5+A4+A3+A2 | Pivot left |
| Mirrored outer/right patterns | Pivot right |
| No black after a left/right sighting | Search toward the last seen side for up to 600 ms, then begin reverse recovery |
| No black and no known side | Begin reverse recovery immediately |
| All five black | Move forward slowly; begin reverse recovery after 250 ms of continuous black |
| Separate black groups, e.g. A5+A1 | Begin reverse recovery with `AMBIGUOUS_LINE` |

The paired-sensor examples assume similar darkness on both sensors. Sharp turns
end as soon as a normal tracking pattern appears. A new visible side immediately
changes turn direction. This version pivots immediately; it has no encoder-based
advance into a corner, route selection or lap counter.
All-black readings are treated as a tracking error; there is no automatic
finish-line stop. Only an explicit STOP or a system fault latches a stop until
RESET. An unsuccessful reverse attempt waits for a normal line to resume.

### เมื่อ error ให้ถอยหลัง

เมื่อเกิด `LINE_LOST`, `AMBIGUOUS_LINE`, `TURN_TIMEOUT` หรือ `ALL_BLACK`
จะเบรก 80 ms แล้วถอยหลังทั้งสองล้อด้วย `REVERSE_SPEED=160` เพื่อหาเส้น โดยไม่ใช้ encoder
คำสั่งจริงหลังชดเชยล้อขวาคือ PWM **-160, -134**; รุ่นก่อนใช้ -110, -92
ซึ่งอาจไม่พอให้มอเตอร์เริ่มหมุนบนพื้นจริง ความสามารถในการออกตัวยังต้องทดสอบบนหุ่น
ถ้าเจอเส้นที่ตามต่อได้ จะเบรกอีก 80 ms ก่อนกลับตามเส้นอัตโนมัติ
เซนเซอร์ริมอย่างเดียว รูปแบบมุม ดำทั้งห้าจุด หรือดำแยกหลายกลุ่ม ยังไม่นับว่ากู้คืนสำเร็จ
จึงไม่กลับไปหมุนซ้ำทันทีเมื่อยังติดอยู่ที่มุมเดิม

ถ้าเส้นหายระหว่างเบรกก่อนเดินหน้า จะถอยต่อโดยใช้เวลาเริ่มถอยเดิม
ไม่เริ่มนับเวลาใหม่ หากยังไม่สำเร็จภายใน `REVERSE_MS=650` ms จะหยุดรอที่
`WAIT_LINE` เพื่อไม่ถอยไปเรื่อย ๆ แต่ยังอ่านเซนเซอร์อยู่ เมื่อเจอเส้นปกติจะเบรก
`BRAKE_MS=80` ms แล้วกลับวิ่งเองถ้ายังเห็นเส้น ไม่ต้อง RESET;
ถ้าไม่เห็นเส้นตอนจบช่วงเบรกจะรอต่อ
การเบรกเพื่อกลับเดินหน้าอาจจบหลังเวลาถอยได้ แต่จะไม่ถอยต่อเกินกำหนด
คำสั่ง `STOP`, ค่าตั้งเซนเซอร์ผิด และ `CONTROL_OVERRUN` ยังหยุดค้างตามเดิม
ไม่เริ่มถอยหรือกลับวิ่งเอง ระยะถอยจริงขึ้นกับความเร็ว แบตเตอรี่ และพื้นสนาม

`RAW` lists A5→A1; `MASK` is hexadecimal, bit 0 = A5. `ERR=-1000` means one sensor
spacing left of center; ordinary nonzero steering error does not trigger recovery.
`LAST_ERROR` records the most recent tracking error, including after recovery.
Recovery states are `ERROR_BRAKE`, `REVERSE`, `LINE_BRAKE` and `WAIT_LINE`.
`PWM` lists signed left/right commands; negative = a reversing wheel during a pivot
or reverse recovery. Reports are snapshots, sent in available Serial buffer space so
output does not wait for a reader. A control update delayed more than 150 ms
while moving causes `CONTROL_OVERRUN`.

## Verification

From the parent `Robot` directory:

```sh
python3 robotline_simple/tests/run_tests.py
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --warnings all robotline_simple
```

Host tests require Python 3 and g++ or clang++. They run the actual sketch with
simulated Arduino I/O and cover steering, direction changes, STOP, search and
turn limits, reverse recovery and resuming after its timeout, STOP in each recovery phase,
black strips, ambiguous lines, clock rollover and busy Serial output.
The `tests/` folder is not part of the Arduino firmware build.

The code is simpler to modify; faster or more reliable lap times have **not** been
measured. Motor traction, sensor height, 90° corner geometry and track lighting
still need tests on the real robot. P-only steering may need a lower speed than
a well-tuned PD controller.
