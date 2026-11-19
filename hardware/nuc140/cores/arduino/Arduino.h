#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "variant.h"
#ifdef __cplusplus
extern "C" {
#endif
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define PA(n) (n)
#define PB(n) (16 + (n))
#define PC(n) (32 + (n))
#define PD(n) (48 + (n))
#define PE(n) (64 + (n))
#define F(x) x
#define bit(b) (1UL << (b))
#define _BV(b) (1UL << (b))
void init(void);
void pinMode(uint32_t pin, uint32_t mode);
void digitalWrite(uint32_t pin, uint32_t value);
int digitalRead(uint32_t pin);
// NUC140 ADC: PA0..PA7 are channels 0..7 (12-bit, 0..4095).
// Returns -1 for unsupported pin or conversion timeout.
int analogRead(uint32_t pin);
void delay(uint32_t ms);
uint32_t millis(void);
void delayMicroseconds(uint32_t us);
void yield(void);
#ifdef __cplusplus
}
#include "HardwareSerial.h"
#endif
