//full duplex

void setup() {
  // put your setup code here, to run once:
  pinMode(13,OUTPUT);

  pinMode(17,INPUT);
  pinMode(18,INPUT);
  pinMode(19,INPUT);
  
  Serial0.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  while(Serial0.available()>0)
  char b = Serial0.read();
  if(b == "1"){
    digitalWrite(13,1);
    delay(250);
  }
  else if(b == "0"){
    digitalWrite(13,0);
  }

  int a = 0;
  int b = 0;
  int c = 0;
  a = digitalRead(17);
  b = digitalRead(18);
  c = digitalRead(19);
  if(a == 1){
    Serial.println("a");
  }
  else if(b == 1){
    Serial.println("b");
  }
  else if(c == 1){
    Serial.println("c");
  }
  else{
    Serial.println("0");
  }
  delay(100);
}
