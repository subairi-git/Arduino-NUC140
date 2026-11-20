#pragma once
#include <Arduino.h>

/* Nu-LB-NUC140 V2.0 common-anode (active LOW) RGB LED.
 * Red PA14 PWM2; Green PA13 PWM1; Blue PA12 PWM0.
 * RGB brightness is 0..255 (0=off, 255=brightest).
 * analogWrite() itself remains Arduino voltage semantics:
 * 0 => LOW, 255 => HIGH.
 */
#define NUC140_RGB_RED    PA(14)
#define NUC140_RGB_GREEN  PA(13)
#define NUC140_RGB_BLUE   PA(12)

static inline void rgbBegin(void) {
  // Set GPIO HIGH (off) before attaching PWM outputs.
  pinMode(NUC140_RGB_RED,OUTPUT);
  pinMode(NUC140_RGB_GREEN,OUTPUT);
  pinMode(NUC140_RGB_BLUE,OUTPUT);
  analogWrite(NUC140_RGB_RED,255);
  analogWrite(NUC140_RGB_GREEN,255);
  analogWrite(NUC140_RGB_BLUE,255);
}

static inline void rgbWrite(uint8_t red, uint8_t green, uint8_t blue) {
  analogWrite(NUC140_RGB_RED,255-(int)red);
  analogWrite(NUC140_RGB_GREEN,255-(int)green);
  analogWrite(NUC140_RGB_BLUE,255-(int)blue);
}
