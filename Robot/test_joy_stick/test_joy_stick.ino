#include "Arduino_LED_Matrix.h"
#include "/home/keaw/github/IOT-Robot/Robot/animation/animation.h"

ArduinoLEDMatrix matrix;

// Left motor
int ENA = 10;
int IN1 = 9;
int IN2 = 8;

// Right motor
int ENB = 5;
int IN3 = 7;
int IN4 = 6;

// Servo
int SV = 11;

// Joystick
int b_stick = 4;
int x_stick = A0;
int y_stick = A1;

int speed_motorL = 0;
int speed_motorR = 0;

int B_L = 0;
int B_R = 0;

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
    if (flip_mode == 0) loadFlippedYFrame(anim[current_frame]);
    else if (flip_mode == 1) loadFlippedXFrame(anim[current_frame]);
    else if (flip_mode == 2) loadFlippedXYFrame(anim[current_frame]);
    current_frame = (current_frame + 1) % total_frames;
  }
}

void setup() {
  Serial.begin(9600);
  
  // Initialize LED matrix
  matrix.begin();

  // Left motor pins setup
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Right motor pins setup
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Servo setup
  pinMode(SV, OUTPUT);

  // Joystick pins setup
  pinMode(b_stick, INPUT_PULLUP);
  pinMode(x_stick, INPUT);
  pinMode(y_stick, INPUT);
}

void forward(int speed_motorL = 200, int speed_motorR = 200, int B_L = 0, int B_R = 15) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);

  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  analogWrite(ENA, speed_motorL - B_R);
  analogWrite(ENB, speed_motorR - B_L);

  int total_frames = sizeof(walk) / sizeof(walk[0]);
  playAnimNonBlocking(walk, total_frames, 0, 1);
}

void backward(int speed_motorL = 200, int speed_motorR = 200, int B_L = 0, int B_R = 15) {
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);

  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);

  analogWrite(ENA, speed_motorL - B_R);
  analogWrite(ENB, speed_motorR - B_L);

  int total_frames = sizeof(walk) / sizeof(walk[0]);
  playAnimNonBlocking(walk, total_frames, 1, 2);
}

void right_turn(int speed_motorR = 200, int speed_motorL = 0, int B_L = 0, int B_R = 0) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  analogWrite(ENA, speed_motorL - B_R);
  analogWrite(ENB, speed_motorR - B_L);

  int total_frames = sizeof(side_walk) / sizeof(side_walk[0]);
  playAnimNonBlocking(side_walk, total_frames, 0, 3);
}

void left_turn(int speed_motorL = 200, int speed_motorR = 0, int B_L = 0, int B_R = 0) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 0);

  analogWrite(ENA, speed_motorL - B_R);
  analogWrite(ENB, speed_motorR - B_L);

  int total_frames = sizeof(side_walk) / sizeof(side_walk[0]);
  playAnimNonBlocking(side_walk, total_frames, 2, 4);
}

void left_turnback(int speed_motorL = 200, int speed_motorR = 200, int B_L = 0, int B_R = 0) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);

  analogWrite(ENA, speed_motorL - B_R);
  analogWrite(ENB, speed_motorR - B_L);

  int total_frames = sizeof(radar) / sizeof(radar[0]);
  playAnimNonBlocking(radar, total_frames, 0, 5);
}

void right_turnback(int speed_motorL = 200, int speed_motorR = 200, int B_L = 0, int B_R = 0) {
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  analogWrite(ENA, speed_motorL - B_R);
  analogWrite(ENB, speed_motorR - B_R);

  int total_frames = sizeof(radar) / sizeof(radar[0]);
  playAnimNonBlocking(radar, total_frames, 2, 6);
}

void stop() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  int total_frames = sizeof(heart) / sizeof(heart[0]);
  playAnimNonBlocking(heart, total_frames, 0, 0);
}

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
  int x_pos = analogRead(x_stick);
  int y_pos = analogRead(y_stick);

  if (y_pos >= 629 && y_pos <= 1023) {
    speed_motorL = map(y_pos, 629, 1023, 100, 255);
    speed_motorR = map(y_pos, 629, 1023, 100, 255);
    Serial.println("Forward");
    forward(speed_motorL, speed_motorR);
  }
  else if (y_pos >= 0 && y_pos <= 460) {
    int inv_y = (460 - y_pos);
    speed_motorL = map(inv_y, 0, 460, 100, 255);
    speed_motorR = map(inv_y, 0, 460, 100, 255);
    Serial.println("Backward");
    backward(speed_motorL, speed_motorR);
  }
  else if (x_pos >= 0 && x_pos <= 460) {
    int inv_x = (460 - x_pos);
    speed_motorR = map(inv_x, 0, 460, 100, 255);
    Serial.println("Right turn");
    right_turn(speed_motorR);
  }
  else if (x_pos >= 629 && x_pos <= 1023) {
    speed_motorL = map(x_pos, 629, 1023, 100, 255);
    Serial.println("Left turn");
    left_turn(speed_motorL);
  }
  else {
    Serial.println("Stop");
    stop();
  }
}

void loop() {
  joy_stick();
}
