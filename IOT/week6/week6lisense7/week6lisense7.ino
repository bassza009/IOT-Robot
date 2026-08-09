void setup() {
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int a = 0;
  int b = 0;

  a = analogRead(A0);
  b = map(a,0,4095,0,255);

  Serial.print("Variable a : ");
  Serial.println(a);
  Serial.print("Variable b : ");
  Serial.println(b);

  analogWrite(2,b);
  delay(50);

}
