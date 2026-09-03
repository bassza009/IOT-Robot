//full duplex

void setup() {
  // put your setup code here, to run once:
  pinMode(13,OUTPUT);
  pinMode(17,OUTPUT);
  Serial0.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  while(Serial0.available()>0)
  char b = Serial0.read();
  if(b == "1"){
    digitalWrite(13,1);
  }
  else if(b == "0"){
    digitalWrite(13,0);
  }

  int a = 0;
  a = digitalRead(17);
  if(a == 11){
    Serial.println("1");
  }
  else if(a == 0){
    Serial.println("0");
  }
  delay(100);
}
