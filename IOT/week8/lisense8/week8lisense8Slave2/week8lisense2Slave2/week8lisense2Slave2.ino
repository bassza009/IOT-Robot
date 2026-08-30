#include<Wire.h>

  //I2C
//D4,D5(Wire1) ,A4,A5(Wire)
//D4,A4 SDA
//D5,A5 CLK
//Master define address to each slave
//Date will look up to address 
//provide role Slave

void setup() {
  // put your setup code here, to run once:
  Wire.begin(9);
  Wire.onReceive(receive_event);
  Serial.begin(9600);
}

void receive_event(int a ){
  While(Wire.avaliable()>0){
    char c = Wire.read();
    Serial.print(c);
    
  }
  digitalWrite(13,1);
  delay(250);
  digitalWrite(13,0);
  delay(250);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  
}
