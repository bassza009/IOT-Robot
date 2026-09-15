# Tamiya Hyper Motor — fast sensor version

A separate Arduino UNO R4 WiFi sketch based on `robotline_simple`.
Existing setting names, calibration values, wiring, P steering and reverse
recovery are retained. The forward base speed starts at PWM 120. Centered
tracking repeats a five-second speed cycle from 120 to 160 and back to 120.
Sharp turns and search pivots begin with a short brake on both wheels.
This version reduces the software delay between seeing a line and steering.

## Open and run

1. Open **robotline_tamiya_hyper_fast.ino** from this folder in Arduino IDE.
2. Select **Arduino UNO R4 WiFi** and edit **Settings.h** as needed.
3. Upload. Motors start at the first sensor scan; there is no START command.
4. Serial Monitor: **115200 baud**, send **STOP** to brake and stay stopped.
   Lowercase works, with or without a newline. Press RESET to run again.

เปิดไฟล์ `.ino` ในโฟลเดอร์นี้ แล้วเลือกบอร์ด UNO R4 WiFi
ปรับตัวแปรใน `Settings.h` รุ่นนี้อ่านเซนเซอร์ต่อเนื่อง
และค่อย ๆ เพิ่ม–ลดกำลังมอเตอร์บนทางตรงตามรอบเวลา

## What changed

| Item | Original | This version by default |
| --- | --- | --- |
| `SAMPLE_MS` | 5 ms wait after a scan | 0: scan every loop |
| `analogRead()` calls per five-sensor scan | 20 | 5 |
| Sensor filtering | Discard first reading, then median of three | One fresh reading per sensor |
| STOP received during scanning | Handled on next loop | Checked again before motor updates |
| Serial timing | None | `SCAN_US` and `GAP_US` |
| Status LED writes | Every control update | When the LED changes |
| Sharp turn / search pivot entry | Immediate pivot | Brake both wheels for 40 ms, then re-evaluate the line |
| Centered forward speed | Constant PWM 160 | Repeat PWM 120 → 160 → 120 over 5 seconds |

Each scan reads A5, A4, A3, A2, A1 in order and immediately calls the existing
steering controller. It uses the installed Arduino ADC API; no ADC clock or
motor PWM frequency is changed. The sensors are read sequentially, not
simultaneously. Five calls instead of twenty does **not** establish a fourfold
speed increase for the whole loop.

## Sensor speed and noise

`SAMPLE_MS = 0` removes the fixed software wait. A positive value restores
an interval in milliseconds after the previous scan. There is no `delay()`.

The new **SENSOR_READS** setting selects the speed/noise tradeoff:

| Value | Reads per sensor | Use |
| --- | --- | --- |
| `1` (default) | One direct reading | Fastest of these three modes |
| `2` | Discard first reading, keep second | If changing ADC channels affects readings |
| `4` | Discard first, median of next three | Original filter; rejects one outlier among the three |

One-reading mode removes the original noise rejection. Check `RAW` and `MASK`
with the motors running. If they flicker while the sensors stay over the same
surface, try `SENSOR_READS = 4` while leaving `SAMPLE_MS = 0`. This retains
filtering and still removes the old 5 ms wait. Motor noise and sensor response
time can limit tracking even with continuous software scans.

ถ้าค่าเซนเซอร์แกว่งเมื่อมอเตอร์หมุน ให้ลอง `SENSOR_READS = 4`
โดยคง `SAMPLE_MS = 0` ไว้ จะกรองสัญญาณแบบเดิมแต่ไม่รอเพิ่ม 5 ms

### Measure on the robot

Serial reports still appear every `REPORT_MS = 100` ms, using available buffer
space. Reports add two values in microseconds:

- **SCAN_US**: time spent reading all five sensors and calculating line position.
- **GAP_US**: time between consecutive scan starts, including the preceding scan,
  steering, motor updates and intervening Serial work.

For example, `GAP_US=500` corresponds to about 2,000 scans/second for that
interval. This is an illustration, not a measured result. These fields are
snapshots, not averages or worst-case measurements. `CONTROL_GAP_MS = 150`
still latches a stop if a control update takes too long while moving.

## วนความเร็วทางตรงรอบละ 5 วินาที

เมื่อเห็นเส้นอยู่กลาง เริ่มรอบดังนี้ แล้ววนรอบใหม่ทันทีเมื่อครบ 5 วินาที:

| เวลาภายในแต่ละรอบ | กำลังพื้นฐานก่อนชดเชยล้อและแก้ทิศ | สถานะใน Serial |
| --- | --- | --- |
| 0–1 วินาที | ค่อย ๆ เพิ่ม PWM 120 → 160 | `STRAIGHT_UP` |
| 1–4 วินาที | คง PWM 160 เป็นเวลา 3 วินาที | `STRAIGHT_MAX` |
| 4–5 วินาที | ค่อย ๆ ลด PWM 160 → 120 | `STRAIGHT_DOWN` |

ถือว่าเส้นอยู่กลางเมื่อเห็น A3 ช่องเดียว หรือ A4+A3+A2 ที่มีความเข้มสมดุล
จน `abs(lineError) <= 0.1` เซนเซอร์ตรวจตำแหน่งเส้นใต้ตัวรถ ไม่ได้วัดความตรง
ของสนามล่วงหน้า หากเส้นออกด้านข้าง จะยกเลิกรอบและใช้ `BASE_SPEED` สำหรับ
แก้โค้งทันที มุมหักยังใช้เบรกก่อนหมุน 40 ms เมื่อกลับมาเห็นเส้นอยู่กลาง
จะเริ่มรอบใหม่จาก PWM 120 เวลาในโค้ง/ช่วงกู้เส้นไม่สะสมในรอบทางตรง

`STRAIGHT_TOTAL_MS = 5000` กำหนดเวลาทั้งรอบ และ `STRAIGHT_MAX_MS = 3000`
กำหนดเวลาคงกำลังสูงสุด เวลาที่เหลือแบ่งให้ช่วงเพิ่มและลดเท่า ๆ กัน
ทุกช่วงใช้เวลา `millis()` โดยไม่หยุดอ่านเซนเซอร์และไม่ใช้ `delay()`
STOP ยังค้างเบรกจนกด RESET ไม่เริ่มรอบใหม่เองหลัง STOP

ตัวเลขเหล่านี้เป็นคำสั่ง PWM ไม่ใช่ความเร็วที่วัดจากล้อหรือระยะทางเป็นเมตร
การเพิ่ม–ลด PWM ไม่รับประกันว่าความเร็วจริงจะเพิ่ม–ลดเป็นเส้นตรงตามเวลา

## Tune Settings.h

| Variable | Value |
| --- | --- |
| `BASE_SPEED` | 120 — start/end of each straight cycle; base speed for ordinary bends |
| `STRAIGHT_MAX_SPEED` | 160 — highest base command within the straight cycle |
| `STRAIGHT_TOTAL_MS` / `STRAIGHT_MAX_MS` | 5000 / 3000 ms |
| `STEERING_KP` | 80 |
| `TURN_SPEED` | 210 |
| `SEARCH_SPEED` / `REVERSE_SPEED` | 160 / 160 |
| `CROSS_SPEED` / `MAX_SPEED` | 110 / 240 |
| `RIGHT_TRIM` | 46800 / 55500 |

PWM values use 0–255. These settings are starting points, not measured
Tamiya Hyper Motor tuning. `STRAIGHT_MAX_SPEED` must be between `BASE_SPEED`
and `MAX_SPEED`; `MAX_SPEED` remains the wheel-output cap, including steering.
Recheck `RIGHT_TRIM` for your motor pair. If the robot
still overshoots corners, reduce `BASE_SPEED` first; reduce `TURN_SPEED` if it
pivots past the line. Faster sampling alone cannot remove mechanical overshoot.

Calibrate `FLOOR_RAW` and `BLACK_RAW` using your actual surfaces and mounting
height. Values remain 12-bit (0–4095); `LINE_THRESHOLD = 450` uses normalized
darkness where floor = 0 and black = 1000.

## Wiring and recovery

| Part | Pins |
| --- | --- |
| Sensors, left → right | A5, A4, A3, A2, A1 |
| Left motor | PWM D5; direction D6 / D7 |
| Right motor | PWM D10; direction D9 / D8 |

Forward uses first direction pin LOW, second HIGH. Zero command brakes with
both HIGH. Normal bends use P steering; outer/corner patterns brake briefly
before pivoting toward the line. A lost line brakes before starting a search
toward the last seen side; if the search expires, the robot brakes and
reverses. Ambiguous lines, turn timeouts and persistent all-black readings
also trigger reverse recovery. After its timeout, `WAIT_LINE` waits for a
normal line to resume. STOP and system faults remain latched until RESET.

### เบรกก่อนหมุนเข้าโค้ง / TURN_BRAKE

`TURN_BRAKE_MS = 40` เป็นเวลาเบรกสองล้อก่อนเริ่มหมุนเข้าโค้งหักหรือหมุนหาเส้น
และก่อนสลับทิศหมุนซ้าย↔ขวา เป็นการชะลอเมื่อเซนเซอร์เห็นรูปแบบมุมแล้ว
ไม่มีเซนเซอร์มองล่วงหน้าเพื่อเบรกก่อนถึงมุมจริง โค้งทั่วไปยังใช้ P steering

ระหว่าง `TURN_BRAKE` ไฟ L ดับ และโค้ดยังอ่านเซนเซอร์และรับ STOP ตลอด
โดยไม่มี `delay()` เมื่อครบเวลา จะใช้เส้นจากการอ่านล่าสุด:

- เส้นอยู่กลาง: กลับตามเส้นทันที
- เส้นอยู่ด้านตรงข้าม: หมุนตามด้านล่าสุด โดยไม่เริ่มนับเวลาเบรกซ้ำ
- เส้นหาย: ใช้การค้นหาตามทิศที่เห็นครั้งล่าสุดและเงื่อนไขหมดเวลาเดิม

การหมุนต่อเนื่องทิศเดิมเบรกครั้งเดียวตอนเริ่ม จึงไม่หยุดซ้ำทุกครั้งที่อ่านเซนเซอร์
เวลาเบรกนับรวมในเวลาค้นหา/เลี้ยว ไม่ยืด deadline เมื่อเบรก
ตั้ง `TURN_BRAKE_MS = 0` เพื่อปิดช่วงเบรกก่อนหมุน

ถ้ายังไหลเข้ามุม ให้ทดลองเพิ่ม 40 เป็น 60 ms; ถ้าชะงักนานให้ลองลดเป็น 20 ms
ค่าเหล่านี้เป็นจุดเริ่มต้นสำหรับทดสอบ ยังไม่ได้วัดระยะหยุดบนรถจริง
โค้ดใช้ขาทิศทางทั้งคู่ HIGH และ enable PWM 255 ซึ่งเป็น Fast motor stop
ตาม [เอกสาร L298 ของ ST, ตาราง 5](https://www.st.com/resource/en/datasheet/l298.pdf)
การเปลี่ยนครั้งนี้เพิ่มช่วงเบรกก่อนหมุน แรงเบรก PWM สูงสุดเท่าเดิม
`BRAKE_MS = 80` ยังใช้เฉพาะการกู้เส้นก่อนถอย/กลับเดินหน้า

## Verification

From this folder:

```sh
python3 tests/run_tests.py
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi --warnings all .
```

Host tests run the real sketch with simulated I/O, including steering, STOP,
search/reverse recovery, clock rollover, busy Serial, sub-millisecond line
changes, scan work, STOP during ADC reads, timing output, ADC overrun and
all three sensor-read modes, turn-brake timing, changes of line during braking,
STOP during turn braking, disabling the turn brake, the repeating five-second
speed cycle, restarting after bends/recovery, and STOP in every speed phase.
Host timings are simulated; the real scan rate
and track performance require measurement on the robot.
