 const int ledPin[4] = {9,10,11,12};

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT); 
  for(int i=0;i<4;i++){
    pinMode(ledPin[i], OUTPUT);
  }
}

void loop() {
  int button;
  int decimal;
  int i=0;
  
  button = digitalRead(2);
  while(button ==0 &&i<=15){
      decimal = i;
      button = digitalRead(2);
      
      Serial.print("I : ");
      Serial.println(i);

      led_blink_binary(decimal >> 0 & 1,ledPin[0]); // >> mean point to array of binary 
      led_blink_binary(decimal >> 1 & 1,ledPin[1]);
      led_blink_binary(decimal >> 2 & 1,ledPin[2]);
      led_blink_binary(decimal >> 3 & 1,ledPin[3]);

      i++;
      delay(1000);
    while(i >15 && button ==0 ){
      
      button = digitalRead(2);
      Serial.println("I no longer than 15");
      
      delay(100);
      
    }
    
  }
  deadled(button);

}

void led_blink_binary(int n,int pin){
  if(n == 1){
    digitalWrite(pin, HIGH);
  }else{
    digitalWrite(pin, LOW);
  }
}

void deadled(int button){
  if(button == 1){
    for(int i=0;i<4;i++){
      digitalWrite(ledPin[i], LOW);
    }
  }
}
