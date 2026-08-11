void setup() {
  Serial.begin(9600);
}

void loop() {
  int count = analogRead(A0);
  int n = map(count, 0, 1023, 0, 255);
  int reverseN = map(count, 0, 1023, 255,0);
  
  analogWrite(A1, n); 
  analogWrite(A2, reverseN); 
  
  
  int a = n;
  
  Serial.print("PWM Value: ");
  Serial.println(a);

  delay(100);
}