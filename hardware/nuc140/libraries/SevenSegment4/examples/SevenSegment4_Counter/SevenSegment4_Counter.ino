#include <SevenSegment4.h>

// Four-digit common-anode 7-segment built into Nu-LB-NUC140 V2.0.
// Scan continues automatically from SysTick; no refresh calls in loop().
void setup() {
  SevenSegment4.begin();
  SevenSegment4.display(1234);
}
void loop() {
  static uint32_t last=0;
  static uint16_t value=1234;
  if (millis()-last >= 1000UL) {
    last=millis();
    value=(uint16_t)((value+1U)%10000U);
    SevenSegment4.display(value);
  }
}
