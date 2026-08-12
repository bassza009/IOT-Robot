const int led[8] = {5,6,7,8,9,10,11,12};
static int i = 0;
void setup() {
  // put your setup code here, to run once:
  for(int i = 0;i<8;i++){
    pinMode(led[i], OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  int n = analogRead(A0);
  int speed = map(n,0,1035,1000,1);
  int previous;
  for(int i =0;i<8;i++){
    digitalWrite(led[previous], LOW);
    digitalWrite(led[i], HIGH);
    previous = i;
    delay(speed);
  }
}
