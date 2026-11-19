/* Nu-LB-NUC140 V2.0: 128x64 LCD preview test over 9-bit SPI3.
   LCD power/backlight via PD14 active LOW, serial UART0 at 115200. */
#include <NUC140LCD.h>
NUC140LCD lcd;

void setup() {
  Serial.begin(115200);
  Serial.println("NUC140LCD SPI3 9-bit TEST");
  if (!lcd.begin()) {
    Serial.print("LCD begin failed, error=");
    Serial.println(lcd.lastError());
    return;
  }
  Serial.println("LCD controller initialized, backlight ON");
  lcd.fill(true);                 // screen all pixels ON (visibility test)
  lcd.display();
  delay(700);
  lcd.clear();
  lcd.rect(0,0,128,64);
  lcd.printText(8,4,"NUC140 LCD TEST");
  lcd.line(7,15,121,15);
  lcd.printText(8,22,"SPI3 9-BIT");
  lcd.printText(8,34,"128 X 64 PIXELS");
  lcd.printText(8,46,"HELLO ARDUINO!");
  bool ok=lcd.display();
  Serial.println(ok ? "LCD framebuffer sent" : "LCD SPI timeout");
  Serial.print("LCD error="); Serial.println(lcd.lastError());
}

void loop() { delay(1000); }
