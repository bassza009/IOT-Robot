void setup() {
  // put your setup code here, to run once:
 // pull up resistor
 // 3.3v -> resistor -> arduino <- switch -> GD
 
 // pull down resistor
// 3.3 v -> switch -> arduino ->resistor -> GD

  Serial.begin(9600);
  pinMode(17,INPUT);
  // pinMode(LED_BUILTIN,OUTPUT); is a pin 13
  pinMode(13,OUTPUT);
  // pinMode(LED_RED,OUTPUT);
  pinMode(14,OUTPUT);
  // pinMode(LED_GREEN,OUTPUT);
  pinMode(15,OUTPUT);
  // pinMode(LED_BLUE,OUTPUT);
  pinMode(16,OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  int a;
  a = digitalRead(17);
  // result button 1 , 0
  if(a == 1){
    digitWrite(13,1);
    led_pat();
  }else {
    digitWrite(13,0);
    digitalWrite(14,1);
    digitalWrite(15,1);
    digitalWrite(16,1);
  }
  delay(10);
}

void led_pat(){
    digitalWrite(14,0); //red
    delay(100);
    digitalWrite(14,1); //red

    digitalWrite(15,0); //green
    delay(100);
    digitalWrite(15,1); //green

    digitalWrite(16,0); //blue
    delay(100);
    digitalWrite(16,1); //blue
    
    
}
