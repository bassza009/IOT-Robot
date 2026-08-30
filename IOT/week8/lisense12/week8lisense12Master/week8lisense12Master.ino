#include<Wire.h>

//I2C
//D4,D5(Wire1) ,A4,A5(Wire)
//D4,A4 SDA
//D5,A5 CLK
//Master define address to each slave
//Date will look up to address 
//provide role Master

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);
  while(!Serial){

  }
  Serial.println();
  Serial.println("scan I2C status :Scanning...");
  byte count = 0;
  byte i = 0;
  Wire.begin();//8-127 define slave address
  for(i = 8;i<119;i++){
    Wire.beginTransmission(i);
    if(Wire.endTransmission() == 0){
      Serial.print("Find device at address :");
      Serial.println(i);
      count++;
      delay(10);
    }
    
  }
  Serial.println("End scanning");
  Serial.print("Found device : ");
  Serial.println("count");
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
