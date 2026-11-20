/*
 * Nu-LB-NUC140 V2.0: GPIO interrupt with onboard SW INT (PB15).
 * No external wiring. Onboard PC12 LED active LOW.
 * Arduino NUC140 v0.1.4 + Interrupt/Timer PREVIEW patch required.
 */
#include <Arduino.h>

const uint32_t SW_INT = PB(15);
const uint32_t LED = PC(12);
volatile uint32_t buttonCount = 0;
volatile uint32_t lastPressMs = 0;

void onButtonFalling() {
  // Interrupt context: NO Serial, no delay, no LCD.
  const uint32_t now = millis();
  if (buttonCount == 0 || (uint32_t)(now - lastPressMs) >= 180UL) {
    lastPressMs = now;
    buttonCount++;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);  // off (active LOW)
  pinMode(SW_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SW_INT), onButtonFalling, FALLING);
  Serial.println("NUC140 SW INT PB15 GPIO INTERRUPT");
  Serial.println("Press onboard SW INT; GPIO ISR counter should increase.");
}

void loop() {
  static uint32_t shown = 0;
  const uint32_t count = buttonCount; // naturally aligned 32-bit atomic read
  if (count != shown) {
    shown = count;
    digitalWrite(LED, (count & 1U) ? LOW : HIGH);
    Serial.print("SW INT presses = ");
    Serial.println(count);
  }
}
