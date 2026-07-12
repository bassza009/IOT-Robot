//Left motor
int ENA = 10;
int IN1 = 9;
int IN2 = 8;
//Right motor
int ENB = 5;
int IN3 = 7;
int IN4 = 6;

int motor_speed = 0;
void setup() {
  // put your setup code here, to run once:
  pinMode(ENA , OUTPUT);
  pinMode(ENB , OUTPUT);
  pinMode(IN1 , OUTPUT);
  pinMode(IN2 , OUTPUT);
  pinMode(IN3 , OUTPUT);
  pinMode(IN4 , OUTPUT);
  Serial.begin(9600);
  
}



void increse_speed(){
    
  if(motor_speed < 100){
    motor_speed = 0;
  }
  for(motor_speed=100;motor_speed<250;motor_speed++){
    Serial.print("Speed :");
    Serial.println(motor_speed);
    digitalWrite(IN1,0);
    digitalWrite(IN2,1);
    analogWrite(ENA,motor_speed);
    digitalWrite(IN3,0);
    digitalWrite(IN4,1);
    analogWrite(ENB,motor_speed);
  }
  
}

void loop() {
  increse_speed();

}
