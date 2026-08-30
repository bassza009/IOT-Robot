#include<Wire.h>
const int led[3] = {17,18,19};
//I2C
//D4,D5(Wire1) ,A4,A5(Wire)
//D4,A4 SDA
//D5,A5 CLK
//Master define address to each slave
//Date will look up to address 
//provide role Master


//pull up switch
void setup() {
  // put your setup code here, to run once:
  Wire.begin();//8-127 define slave address
  for(int i = 17;i<=19;i++){
    pinMode(i,OUTPUT);
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
  int a = 0,b = 0,c = 0;

  a = digitalRead(17);
  b = digitalRead(18);
  c = digitalRead(19);
  if(a == 1){
    Wire.beginTransmission(8);
    Wire.write("University of Phayao");
    Wire.endTransmission();
    delay(500);
  }
  
  if(b == 1){
    Wire.beginTransmission(9);
    Wire.write("Computer engineering");
    Wire.endTransmission();
    delay(500);
  }
  
  if(c == 1){
    Wire.beginTransmission(10);
    Wire.write("Thana Udomseepaiboon");
    Wire.endTransmission();
    delay(500);
  }
  
}
