/*
 * Nu-LB-NUC140 V2.0 LCD SPI3 9-bit: hardware diagnostic.
 * Pin mux PD8=SPI3_SS0, PD9=CLK, PD10=MISO0, PD11=MOSI0;
 * PD14=LCD backlight power (active LOW).
 *
 * Test order: all pixels on 3 s, blank 3 s, text 3 s, repeat.
 * Helps differentiate LCD graphics vs backlight illumination.
 */
#include <NUC140LCD.h>
NUC140LCD lcd;

void showStatus(const char *label) {
  Serial.print(label);
  Serial.print(" display=");
  Serial.print(lcd.display() ? "OK" : "TIMEOUT");
  Serial.print(" lcdError=");
  Serial.println(lcd.lastError());
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("NUC140 LCD SPI3 DIAGNOSTIC v0.1.1");
  Serial.println("SPI3_SS0=PD8 CLK=PD9 MISO=PD10 MOSI=PD11 BL=PD14");
  if (!lcd.begin()) {
    Serial.print("LCD begin failed, lcdError=");
    Serial.println(lcd.lastError());
    return;
  }
  Serial.println("LCD initialized: watch black/clear/text sequence");
}

void loop() {
  lcd.fill(true);
  showStatus("ALL PIXELS ON");
  delay(3000);

  lcd.clear();
  showStatus("ALL PIXELS OFF");
  delay(3000);

  lcd.clear();
  lcd.rect(0, 0, 128, 64);
  lcd.printText(6, 6, "NUC140 LCD TEST");
  lcd.printText(6, 24, "SPI3 9BIT FIX");
  lcd.printText(6, 42, "PD8 SS0");
  showStatus("FRAME + TEXT");
  delay(3000);
}
