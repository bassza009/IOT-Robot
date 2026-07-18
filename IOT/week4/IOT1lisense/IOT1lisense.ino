void setup() {
  // put your setup code here, to run once:
 // pull up resistor
 // 3.3v -> resistor -> arduino <- switch -> GD
 
 // pull down resistor
// 3.3 v -> switch -> arduino ->resistor -> GD

  Serial.begin(9600);
  pinMode(17,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int a;
  a = digitalRead(17);
  // result button 1 , 0
  Serial.println(a);
  delay(10);
}
