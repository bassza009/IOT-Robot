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
  Wire.begin();//8-127 define slave address
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Wire.beginTransmission(8);
  Wire.write("University of Phayao");
  Wire.endTransmission();
  
  Wire.beginTransmission(9);
  Wire.write("Computer engineering");
  Wire.endTransmission();
  
  Wire.beginTransmission(10);
  Wire.write("Thana Udomseepaiboon");
  Wire.endTransmission();
  delay(500);
}
