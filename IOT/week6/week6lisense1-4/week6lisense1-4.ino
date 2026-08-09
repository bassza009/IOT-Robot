/*
0 = 0%
64 = 25%
127 = 50 %
191 = 75%
255 = 100%
*/

/*
led curcuit
D2 -> anode ,ctode -> resister -> GD
*/

void setup() {
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  //first lisense
  analogWrite(2,64);
  //second lisense
  //analogWrite(2,128);
  //third lisense
  //analogWrite(2,191);
  //forth lisense
  //analogWrite(2,255);
  
  delay(50);
}
