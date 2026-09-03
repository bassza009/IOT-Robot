//pull down switch

void setup() {
  // put your setup code here, to run once:
  pinMode(17,INPUT);
  Serial0.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  bool button = digitalRead(17);
  if(button == 1){
    Serial0.println("1");
  }
  else if(button == 0){
    Serial0.println("0");
  }
}
