//pull down switch

void setup() {
  // put your setup code here, to run once:
  
  Serial0.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  int a = 0;
  int b = 0;

  a = analogRead(A0);
  b = map(a,0,4095,0,255);
  Serial0.println(b);
  delay(10);
}
