const int led[8] = {5,6,7,8,9,10,11,12};
const int b[2]={2,3};
static int i = 0 ;
static boolean state; 
void setup() {
  // put your setup code here, to run once:
  for(int i =0;i<8;i++){
    pinMode(led[i], OUTPUT);
  }
  for(int i =0;i<2;i++){
    pinMode(b[i], INPUT_PULLUP);
  }
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int previous;
  int b1 = digitalRead(b[0]);
  int b2 = digitalRead(b[1]);
  if(b1 == 0){
    state = 1;
  }else if(b2 == 0){
    state = 0;
  }
  
  if(state == 0){
    for(int i=0;i<8;i++){
      
      digitalWrite(led[previous],LOW);
      digitalWrite(led[i],HIGH);
      previous = i;
      delay(500);
    }
    
  }else if(state == 1){
    for(int i=8;i>=0;i--){
      
      digitalWrite(led[previous],LOW);
      digitalWrite(led[i],HIGH);
      previous = i;
      delay(500);
    }
  }
  
  
}
