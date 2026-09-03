//full duplex
const int time_d = 100;

void setup() {
  // put your setup code here, to run once:
  pinMode(13,OUTPUT);

  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  
  
  Serial0.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  while(Serial0.available()>0)
  char b = Serial0.read();
  if(b == "a"){
    p1();
    Serial0.println("1");
  }
  else if(b == "b"){
    p2();
    Serial0.println("1");
  }
  else if(b == "c"){
    p3();
    Serial0.println("1");
  }
  else{
  
  digitalWrite(2,0);
  digitalWrite(3,0);
  digitalWrite(4,0);
  digitalWrite(5,0);
  digitalWrite(6,0);
  delay(time_d);

  }
  
  delay(time_d);
}
void p1(){
  
  digitalWrite(2,1);
  delay(time_d);
  digitalWrite(2,0);
  delay(time_d);
  digitalWrite(3,1);
  delay(time_d);
  digitalWrite(3,0);
  delay(time_d);
  digitalWrite(4,1);
  delay(time_d);
  digitalWrite(4,0);
  delay(time_d);
  digitalWrite(5,1);
  delay(time_d);
  digitalWrite(5,0);
  delay(time_d);
  digitalWrite(6,1);
  delay(time_d);
  digitalWrite(6,0);
  delay(time_d);
}

void p2(){

  
  digitalWrite(6,1);
  delay(time_d);
  digitalWrite(6,0);
  delay(time_d);
  digitalWrite(5,1);
  delay(time_d);
  digitalWrite(5,0);
  delay(time_d);
  digitalWrite(4,1);
  delay(time_d);
  digitalWrite(4,0);
  delay(time_d);
  digitalWrite(3,1);
  delay(time_d);
  digitalWrite(3,0);
  delay(time_d);
  digitalWrite(2,1);
  delay(time_d);
  digitalWrite(2,0);
  delay(time_d); 
}

void p3(){
  digitalWrite(2,1);
  delay(time_d);
  digitalWrite(2,0);
  delay(time_d);
  digitalWrite(3,1);
  delay(time_d);
  digitalWrite(3,0);
  delay(time_d);
  digitalWrite(4,1);
  delay(time_d);
  digitalWrite(4,0);
  delay(time_d);
  digitalWrite(5,1);
  delay(time_d);
  digitalWrite(5,0);
  delay(time_d);
  
  digitalWrite(6,1);
  delay(time_d);
  digitalWrite(6,0);
  delay(time_d);
  digitalWrite(5,1);
  delay(time_d);
  digitalWrite(5,0);
  delay(time_d);
  digitalWrite(4,1);
  delay(time_d);
  digitalWrite(4,0);
  delay(time_d);
  digitalWrite(3,1);
  delay(time_d);
  digitalWrite(3,0);
  delay(time_d);
  digitalWrite(2,1);
  delay(time_d);
  digitalWrite(2,0);
  delay(time_d);  
}