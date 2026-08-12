const int led[3]={12,11,10}; //pretend pin 16,15,14

void setup() {
  // put your setup code here, to run once:
  for(int i=0;i<3;i++){
    pinMode(led[i], OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  int a = analogRead(A0);
  int b= analogRead(A1);
  int c= analogRead(A2);
  
  analogWrite(led[0],a);
  analogWrite(led[1],b);
  analogWrite(led[2],c);
}
