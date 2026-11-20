#include <NUC140PWM.h>

void show(const char *name, uint8_t red,uint8_t green,uint8_t blue) {
  rgbWrite(red,green,blue);
  Serial.println(name);
  delay(1200);
}

void setup() {
  Serial.begin(115200);
  rgbBegin();
  Serial.println("NUC140 PWM RGB COLORS TEST");
  Serial.println("PA14=RED PWM2, PA13=GREEN PWM1, PA12=BLUE PWM0");
  Serial.println("Outputs active LOW, hardware PWM about 1kHz");
}
void loop() {
  show("RED",255,0,0);
  show("GREEN",0,255,0);
  show("BLUE",0,0,255);
  show("YELLOW",255,255,0);
  show("CYAN",0,255,255);
  show("MAGENTA",255,0,255);
  show("WHITE",255,255,255);
  show("OFF",0,0,0);
}
