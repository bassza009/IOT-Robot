void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  int d_time = 1000;
  /*----------------------------loop array----------------------------
  int LEDindex[] = {2, 3, 4, 5} ; //index 0 is 2

  int i = 0;
  for (i = 0 ; i < 4 ; i++) {
    Serial.println(i);
    digitalWrite(LEDindex[i], 1);
    delay(d_time);
    digitalWrite(LEDindex[i], 0);
    delay(d_time);
  }
  for (i = 3; i > 0; i--) {
    Serial.println(i);
    digitalWrite(LEDindex[i], 1);
    delay(d_time);
    digitalWrite(LEDindex[i], 0);
    delay(d_time);

  }
  */
  /*-----------ep3-----------
    digitalWrite(2,1);
    digitalWrite(3,0);
    digitalWrite(4,0);
    digitalWrite(5,0);
    delay(d_time);

    digitalWrite(2,0);
    digitalWrite(3,1);
    digitalWrite(4,0);
    digitalWrite(5,0);
    delay(d_time);

    digitalWrite(2,0);
    digitalWrite(3,0);
    digitalWrite(4,1);
    digitalWrite(5,0);
    delay(d_time);

    digitalWrite(2,0);
    digitalWrite(3,0);
    digitalWrite(4,0);
    digitalWrite(5,1);
    delay(d_time);
  */
}
