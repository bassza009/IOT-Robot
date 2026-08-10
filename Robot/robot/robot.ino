#include "Arduino_LED_Matrix.h"
#include "animation.h"
#include "Servo.h"

Servo servo;
ArduinoLEDMatrix matrix;

/*
  analog(0)
  analog(1024) 1.25 V
  analog(2047) 2.5 V
  analog(3071) 3.75 V
  analog(4095) 5 V
  */

//Left motor
int ENA = 10;
int IN1 = 9;
int IN2 = 8;
//Right motor
int ENB = 5;
int IN3 = 7;
int IN4 = 6;

//servo
int SV = 11;

int speed_motorL = 0;
int speed_motorR = 0;

int B_L = 0;
int B_R = 0;

void setup() {
  //analogWriteResolution(12);
  // put your setup code here, to run once:
  Serial.begin(9600);
  matrix.begin();
  //Left motor set up
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  //Right motor set up
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  //Servo
  servo.attach(SV);
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

void forward(int speed_motorL = 200, int speed_motorR = 200, int B_L = 0, int B_R = 15) {
  // Define parameter for L298N


  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);

  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);


  //left motor
  analogWrite(ENA, speed_motorL - B_R);
  //right motor
  analogWrite(ENB, speed_motorR - B_L);
  //animation
  int total_frames = sizeof(walk) / sizeof(walk[0]);
  for (int i = 0; i < total_frames; i++) {
    loadFlippedYFrame(walk[i]);
    delay(walk[i][3]); // 66 ms frame delay
  }
}

void backward(int speed_motorL = 200, int speed_motorR = 200, int B_L = 0, int B_R =15) {
  // Define parameter for L298N
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);

  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);

  //left motor
  analogWrite(ENA, speed_motorL - B_R);
  //right motor
  analogWrite(ENB, speed_motorR - B_L);

  int total_frames = sizeof(walk) / sizeof(walk[0]);
  for (int i = 0; i < total_frames; i++) {
    loadFlippedXFrame(walk[i]);
    delay(walk[i][3]); // 66 ms frame delay
  }
}



void right_turn(int speed_motorR = 200,int speed_motorL = 0, int B_L = 0, int B_R = 0) {


  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  //Define speed for motor
 //left motor
  analogWrite(ENA, speed_motorL - B_R);
  //right motor
  analogWrite(ENB, speed_motorR - B_L);
  Serial.print("Right turn");
  int total_frames = sizeof(side_walk) / sizeof(side_walk[0]);
  for (int i = 0; i < total_frames; i++) {
    loadFlippedYFrame(side_walk[i]);
    delay(side_walk[i][3]); // 66 ms frame delay
  }
}





void left_turn(int speed_motorL = 200, int speed_motorR = 0,int B_L = 0, int B_R = 0) {

  // Define parameter for L298N

  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);

  //Define speed for motor

  //left motor
  analogWrite(ENA, speed_motorL - B_R);
  //right motor
  analogWrite(ENB, speed_motorR - B_L);
  Serial.println("Left turn");

 
  // Redo forward order with swapped X and Y axes
  int total_frames = sizeof(side_walk) / sizeof(side_walk[0]);
  for (int i = 0; i < total_frames; i++) {
    loadFlippedXYFrame(side_walk[i]);
    delay(side_walk[i][3]); // 66 ms frame delay
  }
}

void left_turnback(int speed_motorL = 200, int speed_motorR = 200, int B_L = 0, int B_R = 0) {

  // Define parameter for L298N
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);

  //Define speed for motor
  //left motor
  analogWrite(ENA, speed_motorL - B_R);
  //right motor
  analogWrite(ENB, speed_motorR - B_L);
  int total_frames = sizeof(radar) / sizeof(radar[0]);
  for (int i = 0; i < total_frames; i++) {
    loadFlippedYFrame(radar[i]);
    delay(radar[i][3]); // 66 ms frame delay
  }
}
void right_turnback(int speed_motorL = 200, int speed_motorR = 200, int B_L = 0, int B_R = 0) {
  // Define parameter for L298N
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);

  //Define speed for motor
  //left motor
  analogWrite(ENA, speed_motorL - B_R);
  //right motor
  analogWrite(ENB, speed_motorR - B_R);
  int total_frames = sizeof(radar) / sizeof(radar[0]);
  for (int i = 0; i < total_frames; i++) {
    loadFlippedXYFrame(radar[i]);
    delay(radar[i][3]); // 66 ms frame delay
  }
}
void stop() {
  analogWrite(ENA, 0);

  analogWrite(ENB, 0);
  int total_frames = sizeof(heart) / sizeof(heart[0]);
  for (int i = 0; i < total_frames; i++) {
    loadFlippedYFrame(heart[i]);
    delay(heart[i][3]); // 66 ms frame delay
  }
}

void Sv(int degree) {
  servo.write(degree);
  delay(1000);
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
void loop() {
  // put your main code here, to run repeatedly:
  int d_time = 1000;
  /*
  Sv(180);
  Sv(90);
  Sv(45);
  Sv(0);
  Sv(90);
  */
  
  forward();
  delay(1000);
  backward();
  delay(1000);
  right_turn();
  delay(1000);
  left_turn();
  delay(1000);
  left_turnback();
  delay(1000);
  right_turnback();
  delay(1000);
  stop();
  delay(1000);
  
  
}
