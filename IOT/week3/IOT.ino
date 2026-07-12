void setup() {
  // put your setup code here, to run once:
  //Serial.port(8000)

  //-------------------------------
  //Set pin 2 to OUTPUT mode
  pinMode(2,OUTPUT);

  //--------------------------------
  pinMode(13,OUTPUT);
  pinMode(14, OUTPUT);
//---------------------------------
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("hello world")
  
  //send pin 2 high
  digitalWrite(2,1);
  
  //--------------------------
  digitalWrite(13,0);
  digitalWrite(14,1);
  delay(100);
  
  //send pin 2 low
  digitalWrite(2,0);

  //-----------------------------
  digitalWrite(13,1);
  digitalWrite(14,0);
  delay(100);

  
}
