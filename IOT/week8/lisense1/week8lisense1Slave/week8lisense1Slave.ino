#include<Wire.h>

//I2C
//D4,D5(Wire1) ,A4,A5(Wire)
//D4,A4 SDA
//D5,A5 CLK
//Master define address to each slave
//Date will look up to address 
//provide role slave

void setup() {
  // put your setup code here, to run once:
  Wire.begin(8);//8-127 define slave address
  Wire.onRequest(request_event);
  
}
void request_event(){
  Wire.write("Hello!");//6 bytes(lisense 1 print no ln)
  //Wire.write("Hello\n");//6 bytes(lisense 2 print ln with \n)
  //Wire.write("Hel");//6 bytes(lisense 3 has void byte)
  //Wire.write("University of Phayao\n");//21 bytes(lisense 4 change byte)

}
void loop() {
  // put your main code here, to run repeatedly:
 
  delay(100);
}
