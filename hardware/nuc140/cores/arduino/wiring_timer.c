// NUC140 hardware Timer 0..3, periodic mode, HIRC 22.1184 MHz.
// Nuvoton NUC100BSP register map; timerBegin period is in microseconds.
#include "Arduino.h"
#define MMIO(a) (*(volatile uint32_t *)(a))
#define PROTECT 0x50000100UL
#define PWRCON 0x50000200UL
#define APBCLK 0x50000208UL
#define CLKSTATUS 0x5000020CUL
#define CLKSEL1 0x50000214UL
#define NVIC_ISER 0xE000E100UL
#define NVIC_ICER 0xE000E180UL
#define NVIC_ICPR 0xE000E280UL
#define TIMER_CLOCK 22118400UL
#define CEN (1UL<<30)
#define IE (1UL<<29)
#define PERIODIC (1UL<<27)
#define CRST (1UL<<26)
#define TIF 1UL
static const uint32_t bases[4]={0x40010000UL,0x40010020UL,0x40110000UL,0x40110020UL};
static void (*volatile callbacks[4])(void);
static uint8_t configured[4];
static uint32_t irqLock(void) {
  uint32_t s; __asm__ volatile("mrs %0, primask\n cpsid i" : "=r"(s) :: "memory"); return s;
}
static void irqUnlock(uint32_t s) {__asm__ volatile("msr primask, %0" :: "r"(s) : "memory");}
static void disableIrq(uint8_t i) {MMIO(NVIC_ICER)=1UL<<(i+8);}
static void enableIrq(uint8_t i) {MMIO(NVIC_ISER)=1UL<<(i+8);}
static void clearIrq(uint8_t i) {MMIO(NVIC_ICPR)=1UL<<(i+8);}
static bool dividers(uint32_t period, uint32_t *pre, uint32_t *cmp) {
  if(!period) return false;
  uint64_t total=((uint64_t)TIMER_CLOCK*period+500000ULL)/1000000ULL;
  if(total<2ULL || total>(uint64_t)256*0xFFFFFFUL) return false;
  for(uint32_t d=1;d<=256;d++) {
    uint64_t c=(total+d/2)/d;
    if(c>=2 && c<=0xFFFFFF) { *pre=d-1;*cmp=(uint32_t)c;return true; }
  }
  return false;
}
bool timerBegin(uint8_t i, uint32_t periodUs, void (*callback)(void)) {
  if(i>=4 || !callback) return false;
  uint32_t pre,cmp;
  if(!dividers(periodUs,&pre,&cmp)) return false;
  uint32_t s=irqLock(), base=bases[i];
  disableIrq(i); MMIO(base)=0; callbacks[i]=0; configured[i]=0;
  MMIO(PROTECT)=0x59;MMIO(PROTECT)=0x16;MMIO(PROTECT)=0x88;
  if(!(MMIO(PROTECT)&1)) {irqUnlock(s);return false;}
  MMIO(PWRCON)|=1UL<<2;
  uint32_t timeout=2000000;
  while(!(MMIO(CLKSTATUS)&(1UL<<4)) && timeout) --timeout;
  if(!timeout) {MMIO(PROTECT)=0;irqUnlock(s);return false;}
  MMIO(APBCLK)|=1UL<<(i+2);
  uint32_t shift=8+4*i;
  MMIO(CLKSEL1)=(MMIO(CLKSEL1)&~(7UL<<shift))|(7UL<<shift);
  MMIO(PROTECT)=0;
  MMIO(base+4)=cmp; MMIO(base+8)=TIF;
  MMIO(base)=PERIODIC|IE|pre|CRST;
  MMIO(base)=PERIODIC|IE|pre;
  callbacks[i]=callback;configured[i]=1;
  clearIrq(i);enableIrq(i);MMIO(base)|=CEN;
  irqUnlock(s);return true;
}
bool timerSetPeriod(uint8_t i,uint32_t periodUs) {
  if(i>=4 || !configured[i]) return false;
  uint32_t pre,cmp;
  if(!dividers(periodUs,&pre,&cmp)) return false;
  uint32_t s=irqLock(),base=bases[i], running=MMIO(base)&CEN;
  MMIO(base)=0;MMIO(base+4)=cmp;MMIO(base+8)=TIF;
  MMIO(base)=PERIODIC|(callbacks[i]?IE:0)|pre|CRST;
  MMIO(base)=PERIODIC|(callbacks[i]?IE:0)|pre|running;
  clearIrq(i);irqUnlock(s);return true;
}
bool timerAttachInterrupt(uint8_t i,void (*callback)(void)) {
  if(i>=4 || !configured[i] || !callback) return false;
  uint32_t s=irqLock(),base=bases[i];
  disableIrq(i);MMIO(base)&=~IE;MMIO(base+8)=TIF;
  callbacks[i]=callback;clearIrq(i);enableIrq(i);MMIO(base)|=IE;
  irqUnlock(s);return true;
}
void timerDetachInterrupt(uint8_t i) {
  if(i>=4 || !configured[i]) return;
  uint32_t s=irqLock(),base=bases[i];
  MMIO(base)&=~IE;disableIrq(i);callbacks[i]=0;
  MMIO(base+8)=TIF;clearIrq(i);irqUnlock(s);
}
void timerStart(uint8_t i) {if(i<4 && configured[i]) MMIO(bases[i])|=CEN;}
void timerStop(uint8_t i) {if(i<4 && configured[i]) MMIO(bases[i])&=~CEN;}
void timerEnd(uint8_t i) {
  if(i>=4) return;
  uint32_t s=irqLock();
  disableIrq(i);MMIO(bases[i])=0;MMIO(bases[i]+8)=TIF;
  callbacks[i]=0;configured[i]=0;clearIrq(i);irqUnlock(s);
}
static void dispatch(uint8_t i) {
  uint32_t b=bases[i];
  if(MMIO(b+8)&TIF) {
    MMIO(b+8)=TIF;
    void (*cb)(void)=callbacks[i];
    if(cb) cb();
  }
}
void TMR0_IRQHandler(void) {dispatch(0);}
void TMR1_IRQHandler(void) {dispatch(1);}
void TMR2_IRQHandler(void) {dispatch(2);}
void TMR3_IRQHandler(void) {dispatch(3);}
