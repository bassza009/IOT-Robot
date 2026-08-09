/*
8 bit = 0-255
10 bit = 0-1023
12 bit = 0-4095
*/

void setup() {
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
  //analogWriteResolution(8);
  //lisense 8
  analogWriteResolution(10);
  //lisense 10
  analogWriteResolution(12);
}

void loop() {
  // put your main code here, to run repeatedly:
  //analogWrite(2,128); 
  //lisense 8
  analogWrite(2,128);
  //lisense 9 && 10
  //analogWrite(2,512); //50% of 10 bit res 
  //lisense 11
  //analogWrite(2,2048);
  
  delay(50);
}
