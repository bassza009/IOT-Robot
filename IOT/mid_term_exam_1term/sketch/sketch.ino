void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT); 
  
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
}

void loop() {
  int button;
  int decimal = 0;
  String binary[32];
  int i=0;
  
  button = digitalRead(2);
  while(button ==0 &&i<=15){
    
      button = digitalRead(2);
      digitalWrite(12, HIGH);
      Serial.print("I : ");
      Serial.println(i);
      i++;
      delay(100);
      ledblink(4,i,11);
      ledblink(8,i,10);
      ledblink(11,i,9);
      
    while(i ==15 && button ==0 ){
      
      button = digitalRead(2);
      Serial.println("I no longer than 15");
      
      delay(100);
      
    }
    
  }
  deadled(button);

}

void ledblink(int n,int i,int pin){
  if(i>n){
          digitalWrite(pin, HIGH);
  }
  
}

void deadled(int button){
  if(button == 1){
    digitalWrite(12, LOW);
    digitalWrite(11, LOW);
    digitalWrite(10, LOW);
    digitalWrite(9, LOW);
  }
}