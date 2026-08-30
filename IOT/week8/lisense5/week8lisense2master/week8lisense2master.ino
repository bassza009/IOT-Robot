#include<Wire.h>

  //I2C
//D4,D5(Wire1) ,A4,A5(Wire)
//D4,A4 SDA
//D5,A5 CLK
//Master define address to each slave
//Date will look up to address 
//provide role master

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i =8;i<=10;i++){
    Wire.requestFrom(i,21)//8 is address of slave,6 is bytes of request slave
    while(Wire.avilable()){
      char c = Wire.read()
      Serial.print(c);
    }
    delay(500);
  }
  
  
  
  
}
