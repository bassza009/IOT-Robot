void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial0.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.read() == "1"){
    Serial0.println("Send 1 success!!");
  }
  else if(Serial.read() == "0"){
    Serial0.println("Send 0 success!!");
  }
}
