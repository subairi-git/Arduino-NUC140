/* Nu-LB-NUC140 V2.0: show VR1 ADC7 on onboard 128x64 LCD. */
#include <NUC140LCD.h>
NUC140LCD lcd;

void setup() {
  Serial.begin(115200);
  if (!lcd.begin()) Serial.println("LCD INIT ERROR");
}

void loop() {
  int value=analogRead(PA(7)); // Built-in 20K VR1, 12-bit ADC
  lcd.clear();
  lcd.printText(3,0,"NUC140 VR1 GAUGE");
  lcd.line(0,10,127,10);
  lcd.printText(4,17,"ROTATE VR1 KNOB");
  lcd.rect(0,40,128,14);
  if (value>=0 && value<=4095) {
    int width=(int)((uint32_t)value*124UL/4095UL);
    lcd.fillRect(2,42,width,10);
    Serial.print("VR1 ADC7=");Serial.println(value);
  } else {
    Serial.println("ADC7 error");
  }
  bool ok=lcd.display();
  if (!ok) { Serial.print("LCD SPI error=");Serial.println(lcd.lastError()); }
  delay(300);
}
