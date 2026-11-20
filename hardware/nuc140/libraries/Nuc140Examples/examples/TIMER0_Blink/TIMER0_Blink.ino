/*
 * Nu-LB-NUC140 V2.0: Hardware TIMER0 interrupts every 500000 us.
 * PC12 onboard LED toggles in loop; ISR only increments a counter.
 * Independent of Arduino SysTick/millis(). No external wiring.
 */
#include <Arduino.h>

volatile uint32_t timerTicks = 0;

void onTimer0() {
  timerTicks++; // ISR: keep short
}

void setup() {
  Serial.begin(115200);
  pinMode(PC(12), OUTPUT);
  digitalWrite(PC(12), HIGH);
  if (!timerBegin(0, 500000UL, onTimer0)) {
    Serial.println("ERROR: TIMER0 failed to start");
  } else {
    Serial.println("TIMER0 500ms interrupt started; LED PC12 should blink");
  }
}

void loop() {
  static uint32_t seen = 0;
  uint32_t now = timerTicks;
  if (now != seen) {
    seen = now;
    digitalWrite(PC(12), (now & 1UL) ? LOW : HIGH);
    Serial.print("TIMER0 ticks = ");
    Serial.println(now);
  }
}
