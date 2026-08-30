#include<Wire.h>
#include<rgb_lcd.h>

rgb_lcd lcd;

const int colorR = 0;
const int colorG = ;
const int colorB = 255;

  //I2C
//D4,D5(Wire1) ,A4,A5(Wire)
//D4,A4 SDA
//D5,A5 CLK
//Master define address to each slave
//Date will look up to address 
//provide role master

void setup() {
  // put your setup code here, to run once:
  lcd.begin();
  lcd.setRGB(colorR,colorG,colorB);
  lcd.print("Hello, World");

  delay(1000);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.setCursor(0,0);
  lcd.print("University of Phayao");
  lcd.setCursor(0,1);
  lcd.print("Thana Udomsripaiboon");

  lcd.print(mills()/1000);
  delay(100);
  
  
}
