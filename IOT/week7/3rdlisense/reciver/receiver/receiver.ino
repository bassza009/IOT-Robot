void setup() {
  // put your setup code here, to run once:
  pinMode(13,OUTPUT);
  Serial0.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int b = 0;
  while(Serial0.available()>0)
  {
   b = Serial0.parseInt();
   analogWrite(13,b) 
  }
}
