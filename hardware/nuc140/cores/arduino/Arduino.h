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
void delay(uint32_t ms);
uint32_t millis(void);
void delayMicroseconds(uint32_t us);
void yield(void);
#ifdef __cplusplus
}
#endif
