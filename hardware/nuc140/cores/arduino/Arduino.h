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
#define CHANGE 1
#define FALLING 2
#define RISING 3
#define digitalPinToInterrupt(pin) (pin)
void attachInterrupt(uint32_t pin, void (*callback)(void), int mode);
void detachInterrupt(uint32_t pin);
bool timerBegin(uint8_t index, uint32_t periodUs, void (*callback)(void));
bool timerSetPeriod(uint8_t index, uint32_t periodUs);
bool timerAttachInterrupt(uint8_t index, void (*callback)(void));
void timerDetachInterrupt(uint8_t index);
void timerStart(uint8_t index);
void timerStop(uint8_t index);
void timerEnd(uint8_t index);
// Arduino-compatible tone output, using hardware Timer3.
// Only one tone at a time. Pin must be connected to a passive buzzer.
#ifdef __cplusplus
void tone(uint32_t pin, unsigned int frequency, unsigned long duration = 0);
#else
void tone(uint32_t pin, unsigned int frequency, unsigned long duration);
#endif
void noTone(uint32_t pin);
static inline void noInterrupts(void) { __asm__ volatile("cpsid i" ::: "memory"); }
static inline void interrupts(void) { __asm__ volatile("cpsie i" ::: "memory"); }
void init(void);
void pinMode(uint32_t pin, uint32_t mode);
void digitalWrite(uint32_t pin, uint32_t value);
int digitalRead(uint32_t pin);
// Hardware PWM0..PWM3 on PA12..PA15: 0=LOW, 255=HIGH.
void analogWrite(uint32_t pin, int value);
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
