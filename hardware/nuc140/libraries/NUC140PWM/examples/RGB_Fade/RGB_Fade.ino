#include <NUC140PWM.h>

void fadeChannel(const char *label, uint8_t color) {
  Serial.println(label);
  for(int value=0; value<=255; value+=5) {
    rgbWrite(color==0?value:0,color==1?value:0,color==2?value:0);
    delay(20);
  }
  for(int value=255; value>=0; value-=5) {
    rgbWrite(color==0?value:0,color==1?value:0,color==2?value:0);
    delay(20);
  }
}

void setup() {
  Serial.begin(115200);
  rgbBegin();
  Serial.println("NUC140 RGB PWM FADE, 0..255 brightness");
}
void loop() {
  fadeChannel("RED FADE",0);
  fadeChannel("GREEN FADE",1);
  fadeChannel("BLUE FADE",2);
  Serial.println("ALL-CHANNEL FADE");
  for(int value=0; value<=255; value+=5) {
    rgbWrite(value,value,value);
    delay(20);
  }
  for(int value=255; value>=0; value-=5) {
    rgbWrite(value,value,value);
    delay(20);
  }
  rgbWrite(0,0,0);
  delay(250);
}
