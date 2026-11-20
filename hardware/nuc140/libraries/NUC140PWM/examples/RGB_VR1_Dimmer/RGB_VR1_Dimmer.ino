#include <NUC140PWM.h>

void setup() {
  Serial.begin(115200);
  rgbBegin();
  Serial.println("NUC140 ADC7 PA7 -> PWM0/1/2 RGB dimming");
  Serial.println("Rotate onboard VR1: all RGB channels fade together");
}
void loop() {
  int adc=analogRead(PA(7));
  if (adc<0) {
    Serial.println("ADC ERROR");
    delay(500);
    return;
  }
  uint8_t brightness=(uint8_t)(((uint32_t)adc * 255UL + 2047UL)/4095UL);
  rgbWrite(brightness,brightness,brightness);
  Serial.print("ADC7=");Serial.print(adc);
  Serial.print(" brightness=");Serial.println(brightness);
  delay(100);
}
