 const int ledPin[4] = {9,10,11,12};
 const int button[2] = {2,3};
 int i=0;
void setup() {
  Serial.begin(9600);
  for(int i= 0;i<2;i++){
    pinMode(button[i], INPUT);
  } 
  for(int i=0;i<4;i++){
    pinMode(ledPin[i], OUTPUT);
  }
}

void loop() {
  int button1;
  int button2;
  int decimal;
  
  
  button1 = digitalRead(button[0]);
  button2 = digitalRead(button[1]);
  while(button1 == 0 &&i<=15){
      decimal = i;
      button1 = digitalRead(button[0]);
      
      Serial.print("I : ");
      Serial.println(i);

      led_blink_binary(decimal >> 0 & 1,ledPin[0]); // >> mean point to array of binary 
      led_blink_binary(decimal >> 1 & 1,ledPin[1]);
      led_blink_binary(decimal >> 2 & 1,ledPin[2]);
      led_blink_binary(decimal >> 3 & 1,ledPin[3]);

      i++;
      delay(1000);
    while(i >15 && button1 ==0 ){
      
      button1 = digitalRead(button[0]);
      Serial.println("I no longer than 15");
      
      delay(100);
      
    }
     
  }
  while(button2 == 0 && i >=0){
      decimal = i;
      button2 = digitalRead(button[1]);
      
      Serial.print("I : ");
      Serial.println(i);

      led_blink_binary(decimal >> 0 & 1,ledPin[0]); // >> mean point to array of binary 
      led_blink_binary(decimal >> 1 & 1,ledPin[1]);
      led_blink_binary(decimal >> 2 & 1,ledPin[2]);
      led_blink_binary(decimal >> 3 & 1,ledPin[3]);

      i--;
      delay(1000);  
      while(i <= 0 && button2 ==0 ){
        
        button2 = digitalRead(button[1]);
        Serial.println("I no less than 0");
        
        delay(100);
      
      }
  }
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
