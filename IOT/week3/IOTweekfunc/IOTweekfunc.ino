void setup() {
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  p_led(1,0,0,0);
  p_led(1,0,1,0);
  p_led(1,1,0,0);
  p_led(0,0,1,1);
  p_led(1,0,0,1);
  p_led(0,1,1,0);
}

void p_led(int a, int b, int c, int d){
  int d_time = 250;
  digitalWrite(2,a);
  digitalWrite(3,b);
  digitalWrite(4,c);
  digitalWrite(4,d);
  delay(d_time);
}
