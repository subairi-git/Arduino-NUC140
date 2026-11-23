// NUC140 GPIO interrupts: PA..PE and dedicated PB14/PB15 EINT0/EINT1.
#include "Arduino.h"
#define MMIO(a) (*(volatile uint32_t *)(a))
#define GPIO_BASE 0x50004000UL
#define IMD 0x18UL
#define IEN 0x1CUL
#define ISRC 0x20UL
#define GPB_MFP 0x50000034UL
#define NVIC_ISER 0xE000E100UL
#define NVIC_ICER 0xE000E180UL
#define NVIC_ICPR 0xE000E280UL
static void (*callbacks[80])(void);
static uint8_t saved_mfp[2], mfp_valid[2];
static uint32_t lockIrq(void) {
  uint32_t s;
  __asm__ volatile("mrs %0, primask\n cpsid i" : "=r"(s) :: "memory");
  return s;
}
static void unlockIrq(uint32_t s) { __asm__ volatile("msr primask, %0" :: "r"(s) : "memory"); }
static uint32_t reg(uint32_t port, uint32_t off) { return GPIO_BASE + port*0x40UL + off; }
static uint32_t irqFor(uint32_t pin) {
  if(pin==PB(14)) return 2;
  if(pin==PB(15)) return 3;
  return pin<32 ? 4 : 5;
}
static int hasCallbacks(uint32_t irq) {
  for(uint32_t p=0;p<80;p++) if(callbacks[p] && irqFor(p)==irq) return 1;
  return 0;
}
void attachInterrupt(uint32_t pin, void (*callback)(void), int mode) {
  if(pin>=80 || !callback || (mode!=RISING && mode!=FALLING && mode!=CHANGE)) return;
  uint32_t s=lockIrq(), port=pin/16, n=pin%16, bit=1UL<<n, irq=irqFor(pin);
  MMIO(reg(port,IEN)) &= ~(bit | (bit<<16));
  MMIO(reg(port,IMD)) &= ~bit;
  if(pin==PB(14) || pin==PB(15)) {
    uint32_t slot=n-14;
    if(!mfp_valid[slot]) { saved_mfp[slot]=(MMIO(GPB_MFP)&bit)!=0; mfp_valid[slot]=1; }
    MMIO(GPB_MFP) |= bit;
  }
  MMIO(reg(port,ISRC))=bit;
  callbacks[pin]=callback;
  MMIO(reg(port,IEN)) |= mode==RISING ? (bit<<16) : mode==FALLING ? bit : (bit|(bit<<16));
  MMIO(NVIC_ICPR)=1UL<<irq;
  MMIO(NVIC_ISER)=1UL<<irq;
  unlockIrq(s);
}
void detachInterrupt(uint32_t pin) {
  if(pin>=80) return;
  uint32_t s=lockIrq(), port=pin/16, n=pin%16, bit=1UL<<n, irq=irqFor(pin);
  MMIO(reg(port,IEN)) &= ~(bit | (bit<<16));
  MMIO(reg(port,ISRC))=bit;
  callbacks[pin]=0;
  if(pin==PB(14) || pin==PB(15)) {
    uint32_t slot=n-14;
    if(mfp_valid[slot]) {
      if(saved_mfp[slot]) MMIO(GPB_MFP)|=bit;
      else MMIO(GPB_MFP)&=~bit;
      mfp_valid[slot]=0;
    }
  }
  if(!hasCallbacks(irq)) MMIO(NVIC_ICER)=1UL<<irq;
  unlockIrq(s);
}
static void dispatch(uint32_t port, uint32_t mask) {
  uint32_t active=MMIO(reg(port,ISRC))&mask;
  if(!active) return;
  MMIO(reg(port,ISRC))=active;
  for(uint32_t n=0;n<16;n++) if(active&(1UL<<n)) {
    void (*cb)(void)=callbacks[port*16+n];
    if(cb) cb();
  }
}
void EINT0_IRQHandler(void) {dispatch(1,1UL<<14);}
void EINT1_IRQHandler(void) {dispatch(1,1UL<<15);}
void GPAB_IRQHandler(void) {dispatch(0,0xFFFF);dispatch(1,0x3FFF);}
void GPCDEF_IRQHandler(void) {dispatch(2,0xFFFF);dispatch(3,0xFFFF);dispatch(4,0xFFFF);}
