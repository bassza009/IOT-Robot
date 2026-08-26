#include "Arduino_LED_Matrix.h"
#include "animation.h"

ArduinoLEDMatrix matrix;

// Left motor pins (L298N)
int ENA = 10;
int IN1 = 9;
int IN2 = 8;

// Right motor pins (L298N)
int ENB = 5;
int IN3 = 7;
int IN4 = 6;

// Servo pin
int SV = 11;

// Joystick pins
int b_stick = 4;   // Digital pin for joystick button
int x_stick = A0;  // Analog pin for X axis
int y_stick = A1;  // Analog pin for Y axis

// Motor speed & calibration offsets
int speed_motorL = 0;
int speed_motorR = 0;
int B_L = 0;   // Left motor balance offset
int B_R = 15;  // Right motor balance offset

// Non-blocking animation state
unsigned long last_frame_time = 0;
int current_frame = 0;
int current_anim_id = -1;

void playAnimNonBlocking(const uint32_t anim[][4], int total_frames, int flip_mode, int anim_id) {
  unsigned long now = millis();
  if (current_anim_id != anim_id) {
    current_anim_id = anim_id;
    current_frame = 0;
    last_frame_time = 0;
  }
  
  if (now - last_frame_time >= 66) { // 66 ms frame delay
    last_frame_time = now;
    if (flip_mode == 0) {
      loadFlippedYFrame(anim[current_frame]);
    } else if (flip_mode == 1) {
      loadFlippedXFrame(anim[current_frame]);
    } else if (flip_mode == 2) {
      loadFlippedXYFrame(anim[current_frame]);
    }
    current_frame = (current_frame + 1) % total_frames;
  }
}

void setup() {
  Serial.begin(9600);

  // Initialize LED Matrix
  matrix.begin();

  // Left motor set up
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Right motor set up
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Servo set up
  pinMode(SV, OUTPUT);

  // Joystick pins setup
  // Use INPUT_PULLUP to prevent floating state on the button pin
  pinMode(b_stick, INPUT_PULLUP);
  pinMode(x_stick, INPUT);
  pinMode(y_stick, INPUT);
}

void increse_speed() {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  for (int i = 2050; i < 4096; i++) {
    analogWrite(ENA, i - B_L);
    analogWrite(ENB, i - B_R);
  }
  for (int i = 4095; i > 2051; i--) {
    analogWrite(ENA, i - B_L);
    analogWrite(ENB, i - B_R);
  }
}

void forward(int speed_motorL = 200, int speed_motorR = 200, int b_l = 0, int b_r = 15) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  analogWrite(ENA, constrain(speed_motorL - b_r, 0, 255));
  analogWrite(ENB, constrain(speed_motorR - b_l, 0, 255));

  int total_frames = sizeof(walk) / sizeof(walk[0]);
  playAnimNonBlocking(walk, total_frames, 0, 1);
}

void backward(int speed_motorL = 200, int speed_motorR = 200, int b_l = 0, int b_r = 15) {
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);

  analogWrite(ENA, constrain(speed_motorL - b_r, 0, 255));
  analogWrite(ENB, constrain(speed_motorR - b_l, 0, 255));

  int total_frames = sizeof(walk) / sizeof(walk[0]);
  playAnimNonBlocking(walk, total_frames, 1, 2);
}

void right_turn(int speed_motorR = 200, int speed_motorL = 0, int b_l = 0, int b_r = 0) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  analogWrite(ENA, constrain(speed_motorL - b_r, 0, 255));
  analogWrite(ENB, constrain(speed_motorR - b_l, 0, 255));

  int total_frames = sizeof(side_walk) / sizeof(side_walk[0]);
  playAnimNonBlocking(side_walk, total_frames, 0, 3);
}

void left_turn(int speed_motorL = 200, int speed_motorR = 0, int b_l = 0, int b_r = 0) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 0);

  analogWrite(ENA, constrain(speed_motorL - b_r, 0, 255));
  analogWrite(ENB, constrain(speed_motorR - b_l, 0, 255));

  int total_frames = sizeof(side_walk) / sizeof(side_walk[0]);
  playAnimNonBlocking(side_walk, total_frames, 2, 4);
}

void left_turnback(int speed_motorL = 200, int speed_motorR = 200, int b_l = 0, int b_r = 0) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);

  analogWrite(ENA, constrain(speed_motorL - b_r, 0, 255));
  analogWrite(ENB, constrain(speed_motorR - b_l, 0, 255));

  int total_frames = sizeof(radar) / sizeof(radar[0]);
  playAnimNonBlocking(radar, total_frames, 0, 5);
}

void right_turnback(int speed_motorL = 200, int speed_motorR = 200, int b_l = 0, int b_r = 0) {
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  analogWrite(ENA, constrain(speed_motorL - b_r, 0, 255));
  analogWrite(ENB, constrain(speed_motorR - b_r, 0, 255));

  int total_frames = sizeof(radar) / sizeof(radar[0]);
  playAnimNonBlocking(radar, total_frames, 2, 6);
}

void stop() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  int total_frames = sizeof(heart) / sizeof(heart[0]);
  playAnimNonBlocking(heart, total_frames, 0, 0);
}

// Transformation functions for LED Matrix
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

void joy_stick() {
  // Read analog values
  int x_pos = analogRead(x_stick);
  int y_pos = analogRead(y_stick);

  // Deadzone range: 450 - 580 (neutral center)
  // Forward: Y is pushed forward
  if (y_pos >= 580 && y_pos <= 1023) {
    speed_motorL = map(y_pos, 580, 1023, 120, 255);
    speed_motorR = map(y_pos, 580, 1023, 120, 255);
    Serial.println("Forward");
    forward(speed_motorL, speed_motorR, B_L, B_R);
  }
  // Backward: Y is pulled backward
  else if (y_pos >= 0 && y_pos <= 450) {
    int inv_y = (450 - y_pos);
    speed_motorL = map(inv_y, 0, 450, 120, 255);
    speed_motorR = map(inv_y, 0, 450, 120, 255);
    Serial.println("Backward");
    backward(speed_motorL, speed_motorR, B_L, B_R);
  }
  // Left turn: X is pushed left
  else if (x_pos >= 0 && x_pos <= 450) {
    int inv_x = (450 - x_pos);
    speed_motorL = map(inv_x, 0, 450, 120, 255);
    Serial.println("Left turn");
    left_turn(speed_motorL, 0, B_L, B_R);
  }
  // Right turn: X is pushed right
  else if (x_pos >= 580 && x_pos <= 1023) {
    speed_motorR = map(x_pos, 580, 1023, 120, 255);
    Serial.println("Right turn");
    right_turn(speed_motorR, 0, B_L, B_R);
  }
  // Neutral / Disconnected / Deadzone -> Stop
  else {
    Serial.println("Stop");
    stop();
  }
}

void loop() {
  joy_stick();
}

