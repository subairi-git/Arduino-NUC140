#include <Arduino.h>
volatile uint32_t presses=0;
void onPress() { ++presses; }
void setup() {
  Serial.begin(115200);
  pinMode(PB(15), INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PB(15)), onPress, FALLING);
  Serial.println("PB15 GPIO interrupt test");
}
void loop() {
  static uint32_t last=0;
  uint32_t n=presses;
  if(n!=last) { last=n; Serial.print("PB15 presses: "); Serial.println(n); }
}
