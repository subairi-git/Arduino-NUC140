#include "Arduino.h"
// NUC100/NUC140 GPIO register map, no third-party BSP needed.
#define REG32(addr) (*(volatile uint32_t *)(addr))
#define GPIO_BASE 0x50004000UL
#define GPIO_PORT_SIZE 0x40UL
#define SYST_CSR REG32(0xE000E010UL)
#define SYST_RVR REG32(0xE000E014UL)
#define SYST_CVR REG32(0xE000E018UL)
static volatile uint32_t tick_ms;
static volatile uint32_t fallback_loops;
// Optional 7-segment scan hook: linked only when SevenSegment4 library is used.
void sevenSegment4Tick(void) __attribute__((weak));
void SysTick_Handler(void) {
  ++tick_ms;
  if (sevenSegment4Tick) sevenSegment4Tick();
}
void init(void) {
  // Expected factory reset HCLK source: 22.1184MHz HIRC / 1.
  // This clock assumption MUST be confirmed against device configuration words.
  tick_ms=0;
  SYST_RVR=(22118400UL/1000UL)-1UL;
  SYST_CVR=0;
  SYST_CSR=7; // enabled, exception enabled, CPU clock
}
void pinMode(uint32_t pin, uint32_t mode) {
  uint32_t port=pin/16U, n=pin%16U;
  if(port>4) return;
  volatile uint32_t *pmd=(volatile uint32_t *)(GPIO_BASE+port*GPIO_PORT_SIZE);
  uint32_t mask=3UL << (2U*n);
  uint32_t value=(mode==OUTPUT ? 1UL : mode==INPUT_PULLUP ? 3UL : 0UL) << (2U*n);
  *pmd=(*pmd & ~mask) | value;
  if(mode==INPUT_PULLUP) digitalWrite(pin,HIGH);
}
void digitalWrite(uint32_t pin,uint32_t value) {
  uint32_t port=pin/16U,n=pin%16U;
  if(port>4) return;
  volatile uint32_t *dout=(volatile uint32_t *)(GPIO_BASE+port*GPIO_PORT_SIZE+0x08UL);
  if(value) *dout |= 1UL << n; else *dout &= ~(1UL << n);
}
int digitalRead(uint32_t pin) {
  uint32_t port=pin/16U,n=pin%16U;
  if(port>4) return LOW;
  return (REG32(GPIO_BASE+port*GPIO_PORT_SIZE+0x10UL)&(1UL<<n))?HIGH:LOW;
}
uint32_t millis(void) { return tick_ms; }
void delay(uint32_t ms) {
 uint32_t start=millis();
 while ((uint32_t)(millis()-start)<ms) yield();
}
void delayMicroseconds(uint32_t us) {
 // Approximate busy-wait; for accurate timing use a hardware timer.
 while(us--) { for(volatile uint32_t n=0;n<3;n++) fallback_loops++; }
}
__attribute__((weak)) void yield(void) {}
