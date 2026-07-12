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

//Pin that receive from variable resistor
int analogIP_L = A0;
int analogIP_R = A1;

//speed motor
int speed_motorL = 0;
int speed_motorR = 0;
// Deceleration
int B_L = 0;
int B_R = 0;

void setup() {
  //analogWriteResolution(12);
  // put your setup code here, to run once:
  Serial.begin(9600);
//Left motor set up
  pinMode(ENA,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  //Right motor set up
  pinMode(ENB,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  //Servo
  pinMode(SV,OUTPUT);
  
}

void increse_speed(){
  digitalWrite(IN1,0);
  digitalWrite(IN2,1);
  digitalWrite(IN3,0);
  digitalWrite(IN4,1);

  for(int i = 2050;i<4096;i++){
    analogWrite(ENA,i-B_L);
    analogWrite(ENB,i-B_R);
  }
  for(int i = 4095;i>2051;i--){
    analogWrite(ENA,i-B_L);
    analogWrite(ENB,i-B_R);
  }
}

void  forward(int speed_motorL = 200,int speed_motorR = 200,int B_L = 0,int B_R = 0){
  // Define parameter for L298N
 
  
  digitalWrite(IN1,0);
  digitalWrite(IN2,1);
  
  digitalWrite(IN3,0);
  digitalWrite(IN4,1);


  //left motor
  analogWrite(ENA,speed_motorL-B_R);
  //right motor
  analogWrite(ENB,speed_motorR-B_L);
  
}

void backward(int speed_motorL = 200,int speed_motorR = 200,int B_L = 0,int B_R = 0){
  
  // Define parameter for L298N 
  digitalWrite(IN1,1);
  digitalWrite(IN2,0);
  
  digitalWrite(IN3,1);
  digitalWrite(IN4,0);

  //Define speed for motor
  //left motor
  analogWrite(ENA,speed_motorL-B_R);
  //right motor
  analogWrite(ENB,speed_motorR-B_L);
 
}

void right_turn(int speed_motorR = 200,int B_L = 0,int B_R = 0,int degree){
  
  
  digitalWrite(IN3,0);
  digitalWrite(IN4,1);

  //Define speed for motor
  
  //right motor
  analogWrite(ENB,speed_motorR-B_L);
  delay(degree);

}

void left_turn(int speed_motorL = 200,int B_L = 0,int B_R = 0,int degree){
  
  // Define parameter for L298N 
  
  digitalWrite(IN1,0);
  digitalWrite(IN2,1);

  //Define speed for motor
  
  //right motor
  analogWrite(ENA,speed_motorL-B_R );
  delay(degree);

}

void left_turnback(int speed_motorL = 200,int speed_motorR = 200,int B_L = 0,int B_R = 0){
  
  // Define parameter for L298N 
  digitalWrite(IN1,0);
  digitalWrite(IN2,1);
  digitalWrite(IN3,1);
  digitalWrite(IN4,0);

  //Define speed for motor
  //left motor
  analogWrite(ENA,speed_motorL-B_R);
  //right motor
  analogWrite(ENB,speed_motorR-B_L);
  

}
void right_turnback(int speed_motorL = 200,int speed_motorR = 200,int B_L = 0,int B_R = 0){
  // Define parameter for L298N 
  digitalWrite(IN1,1);
  digitalWrite(IN2,0);
  digitalWrite(IN3,0);
  digitalWrite(IN4,1);

  //Define speed for motor
  //left motor
  analogWrite(ENA,speed_motorL-B_R);
  //right motor
  analogWrite(ENB,speed_motorR-B_R);
  
}
void stop(){
  analogWrite(ENA,0);
  
  analogWrite(ENB,0);
  delay(1000);
}

void  servo(int degree){
  //digitalWrite(SV, 1);
  analogWrite(SV,degree);
}

void loop() {
  // put your main code here, to run repeatedly:
  int d_time = 1000; 

  //Read from variable resistor
  speed_motorL = analogRead(analogIP_L);
  speed_motorR = analogRead(analogIP_R);
  //Map variable
  speed_motorL = map(speed_motorL,0,100,0,250);
  speed_motorR  = map(speed_motorR,0,100,0,250);

  if(speed_motorL < 150){
    speed_motorL = 0;
  }
  if(speed_motorR < 150){
    speed_motorL = 0;
  }
  
  forward(1000,1000);
  delay(1000);
  stop();
  /*
  backward();
  delay(1000);
  stop();
  right_turn();
  delay(500);
  stop();
  left_turn();
  delay(500);
  stop();
  right_turnback();
  delay(500);
  stop();
  left_turnback();
  delay(500);
  stop();
  
  */
  
}
