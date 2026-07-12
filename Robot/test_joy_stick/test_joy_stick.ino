/*quater
  y+ 567 - 1023
  x- 0 - 460
  x+ 564 - 1023
  y- 460 - 0
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

//joy stick
int b_stick = 4;
int x_stick = A0;
int y_stick = A1;
//joy stick position
int x_pos = 512;
int y_pos = 512;
bool b_button = 0;

int speed_motorL = 0;
int speed_motorR = 0;

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

  //join stick
  pinMode(b_stick,INPUT);
  pinMode(x_stick,INPUT);
  pinMode(y_stick,INPUT);
  
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

void right_turn(int speed_motorR = 200,int B_L = 0,int B_R = 0){
  
  
  digitalWrite(IN3,0);
  digitalWrite(IN4,1);

  //Define speed for motor
  
  //right motor
  analogWrite(ENB,speed_motorR-B_L);
  

}

void left_turn(int speed_motorL = 200,int B_L = 0,int B_R = 0){
  
  // Define parameter for L298N 
  
  digitalWrite(IN1,0);
  digitalWrite(IN2,1);

  //Define speed for motor
  
  //right motor
  analogWrite(ENA,speed_motorL-B_R );
  

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
  //Servo(SV,degree);
}
/*quater
  y+ 567 - 1023
  x- 0 - 460
  x+ 564 - 1023
  y- 460 - 0
*/

void joy_stick(){
  int x_pos = analogRead(x_stick);
  int y_pos = analogRead(y_stick);
  b_button = analogRead(b_stick);
    if(((y_pos >= 629)&&(y_pos <=1023))&&b_button == 1){
      speed_motorL = map(y_pos,629,1023,0,255);
      speed_motorR = map(y_pos,629,1023,0,255);
      Serial.println("Forward");
      forward(speed_motorL,speed_motorR);
    }
    else if(((y_pos  >= 0)&&(y_pos <=460))&&b_button == 1){
      y_pos = (y_pos -460)*(-1);
      speed_motorL = map(y_pos,0,460,0,255);
      speed_motorR = map(y_pos,0,460,0,255);
      Serial.println("backward");
      backward();
    }
    else if(((x_pos >= 0)&&(x_pos<=460))&&b_button == 1){
      Serial.println("right turn");
      right_turn();
    }
    else if(((x_pos >= 629)&&(x_pos<=1023))&&b_button == 1){
      Serial.println("left turn");
      left_turn();
    }
    
    else{
      Serial.println("stop");
     stop(); 
     }
    
}

void loop() {
  // put your main code here, to run repeatedly:
  int d_time = 1000; 
  Serial.print("x :");
  Serial.println(analogRead(x_stick));
  Serial.print("y :");
  Serial.println(analogRead(y_stick));
  
  
  joy_stick();
    

  
  /*
  forward(1000,1000);
  delay(1000);
  */
}
