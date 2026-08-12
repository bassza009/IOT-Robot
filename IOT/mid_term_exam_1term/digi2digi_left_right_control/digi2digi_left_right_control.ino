const int led[10] ={5,6,7,8,9,10,11,12,14,15};
const int b[2] = {2,3};
static int i=0;

void setup() {
  // put your setup code here, to run once:
  for(int i=0;i <10;i++){
    pinMode(led[i], OUTPUT);
  }
  for(int i = 0;i<2;i++){
    pinMode(b[i], INPUT_PULLUP);
  }
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  int b1 = digitalRead(b[0]);
  int b2 = digitalRead(b[1]);
  int previous;
  
  while(b1 == 0 &&b2 ==1&&i<=7){
    digitalWrite(led[previous], LOW);
    digitalWrite(led[i], HIGH);
    previous = i;
    Serial.print("I :");
    Serial.println(i);
    if(i==7){
      digitalWrite(led[8], HIGH);
    }else{
      digitalWrite(led[8], LOW);
    }
    i++;
    delay((100));
    
  
  }
  while(b2 == 0 &&b1 ==1&&i>=0){
    digitalWrite(led[previous], LOW);
    digitalWrite(led[i], HIGH);
    previous = i;
    Serial.print("I :");
    Serial.println(i);
    if(i==0){
      digitalWrite(led[9], HIGH);
    }else{
      digitalWrite(led[9], LOW);
    }
    i--;
    delay((100));
    
  }
}
