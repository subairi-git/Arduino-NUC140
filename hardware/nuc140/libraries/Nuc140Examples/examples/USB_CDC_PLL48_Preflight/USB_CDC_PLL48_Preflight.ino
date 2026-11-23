#include <Arduino.h>
// Stage 2: prepare stable 48MHz PLL for USB without switching CPU HCLK.
// Does not enable USBD and does not create a COM port.
#define REG32(a) (*(volatile uint32_t *)(a))
#define SYS_REGWRPROT 0x50000100UL
#define CLK_PWRCON   0x50000200UL
#define CLK_CLKSTATUS 0x5000020CUL
#define CLK_CLKSEL0  0x50000210UL
#define CLK_PLLCON   0x50000220UL
#define HXT_READY  (1UL << 0)
#define PLL_READY  (1UL << 2)
#define HXT_ENABLE (1UL << 0)
#define PLL_48MHZ_FROM_12MHZ 0x0000C22EUL
static void printHex(const char *name, uint32_t value) {
  Serial.print(name); Serial.print(" = 0x"); Serial.println(value, HEX);
}
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("USB CDC CLOCK 48MHz PREFLIGHT");
  uint32_t originalClockSel=REG32(CLK_CLKSEL0);
  uint32_t originalPll=REG32(CLK_PLLCON);
  uint32_t originalStatus=REG32(CLK_CLKSTATUS);
  printHex("CLKSEL0 before",originalClockSel);
  printHex("PLLCON before",originalPll);
  printHex("CLKSTATUS before",originalStatus);
  if((originalClockSel & 7UL)==2UL || (originalStatus & PLL_READY)) {
    Serial.println("SKIP: PLL is already in use. Do not override.");
    return;
  }
  noInterrupts();
  REG32(SYS_REGWRPROT)=0x59UL;
  REG32(SYS_REGWRPROT)=0x16UL;
  REG32(SYS_REGWRPROT)=0x88UL;
  bool unlocked=(REG32(SYS_REGWRPROT)&1UL)!=0;
  if(unlocked) {
    REG32(CLK_PWRCON)|=HXT_ENABLE;
  }
  REG32(SYS_REGWRPROT)=0UL;
  interrupts();
  if(!unlocked) {
    Serial.println("FAIL: protected register unlock");
    return;
  }
  uint32_t start=millis();
  while(!(REG32(CLK_CLKSTATUS)&HXT_READY) && millis()-start<1500UL) {}
  if(!(REG32(CLK_CLKSTATUS)&HXT_READY)) {
    Serial.println("FAIL: 12MHz HXT not stable");
    return;
  }
  // Nuvoton BSP CLK_EnablePLL() default HXT 48MHz PLLCON = 0xC22E.
  // Keep CLKSEL0 unchanged, so HCLK/SysTick/UART remain on their old clocks.
  noInterrupts();
  REG32(SYS_REGWRPROT)=0x59UL;
  REG32(SYS_REGWRPROT)=0x16UL;
  REG32(SYS_REGWRPROT)=0x88UL;
  unlocked=(REG32(SYS_REGWRPROT)&1UL)!=0;
  if(unlocked) REG32(CLK_PLLCON)=PLL_48MHZ_FROM_12MHZ;
  REG32(SYS_REGWRPROT)=0UL;
  interrupts();
  if(!unlocked) {
    Serial.println("FAIL: PLL register unlock");
    return;
  }
  start=millis();
  while(!(REG32(CLK_CLKSTATUS)&PLL_READY) && millis()-start<1500UL) {}
  printHex("PLLCON after",REG32(CLK_PLLCON));
  printHex("CLKSEL0 after",REG32(CLK_CLKSEL0));
  printHex("CLKSTATUS after",REG32(CLK_CLKSTATUS));
  if((REG32(CLK_CLKSTATUS)&PLL_READY) &&
      REG32(CLK_PLLCON)==PLL_48MHZ_FROM_12MHZ &&
      REG32(CLK_CLKSEL0)==originalClockSel) {
    Serial.println("PASS: 48MHz PLL ready, CPU clock unchanged");
  } else {
    Serial.println("FAIL: verify PLL clock status / CLKSEL0");
  }
  Serial.println("USB CDC not started yet; Windows COM port is not expected.");
}
void loop() {
  static uint32_t last=0;
  if(millis()-last>=1000UL) {
    last=millis();
    Serial.print("alive millis="); Serial.println(last);
  }
}
