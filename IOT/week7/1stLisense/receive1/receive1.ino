void setup() {
  // put your setup code here, to run once:
  pinMode(13,OUTPUT);
  Serial0.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial0.available()>0)
{
  char a = Serial0.read();
  if(a == "1"){
    digitalWrite(13,HIGH);
  }
  else if(a == "0"){
    digitalWrite(13,LOW);
  }
}}
