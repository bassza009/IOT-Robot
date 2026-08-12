const int led[9] = {5,6,7,8,9,10,11,12,13};

void setup() {
  // put your setup code here, to run once:

  
  for(int i = 0;i <9;i++){
    pinMode(led[i], OUTPUT);
  }
  
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int count;
  count = analogRead(A0);
  
  int n;
  
  n = map(count,0,1023,0,8);//1023 is max of my example change to max of your variable resistor
  
  
  for(int i = 0;i<9;i++){
    if(i < n){
      digitalWrite(led[i], HIGH);
      if(n==8){
        digitalWrite(led[8], HIGH);
      }else{
        digitalWrite(led[8], LOW);
    }
    }
    else{
      digitalWrite(led[i], LOW);
    }
  }

  Serial.print("N : ");
  Serial.println(n);
  
  if(n == 8)
  delay(100);
}
