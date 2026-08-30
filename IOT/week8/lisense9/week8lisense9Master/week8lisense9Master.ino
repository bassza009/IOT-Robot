#include<Wire.h>
const int led[3] = {17,18,19};
//I2C
//D4,D5(Wire1) ,A4,A5(Wire)
//D4,A4 SDA
//D5,A5 CLK
//Master define address to each slave
//Date will look up to address 
//provide role Master
byte x = 0;

//pull up switch
void setup() {
  // put your setup code here, to run once:
  Wire.begin();//8-127 define slave address

  
}

void loop() {
  // put your main code here, to run repeatedly:
  Wire.beginTransmission(8);
  Wire.write(x);
  Wire.endTransmission();
  
  x++;
  
  delay(500);
  
}
