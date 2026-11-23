#include <Arduino.h>
// USB CDC hardware clock preflight for Nu-LB-NUC140 V2.0.
// This is NOT a USB CDC driver and will NOT create a COM port.
// 12MHz external crystal is needed for the future USB 48MHz PLL clock.
#define REG32(addr) (*(volatile uint32_t *)(addr))
#define SYS_REGWRPROT 0x50000100UL
#define CLK_PWRCON 0x50000200UL
#define CLK_CLKSTATUS 0x5000020CUL
#define HXT_ENABLE (1UL << 0)
#define HXT_READY  (1UL << 0)
#define LED PC(12)
void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  Serial.begin(115200);
  delay(500);
  Serial.println("NUC140 USB CDC PRE-FLIGHT (not CDC yet)");
  Serial.print("PWRCON before = 0x");
  Serial.println(REG32(CLK_PWRCON), HEX);
  Serial.print("CLKSTATUS before = 0x");
  Serial.println(REG32(CLK_CLKSTATUS), HEX);
  noInterrupts();
  REG32(SYS_REGWRPROT)=0x59;
  REG32(SYS_REGWRPROT)=0x16;
  REG32(SYS_REGWRPROT)=0x88;
  bool unlocked=(REG32(SYS_REGWRPROT)&1UL)!=0;
  if(unlocked) REG32(CLK_PWRCON)|=HXT_ENABLE;
  REG32(SYS_REGWRPROT)=0;
  interrupts();
  Serial.print("Protected registers unlocked: ");
  Serial.println(unlocked ? "YES":"NO");
  uint32_t t0=millis();
  while(!(REG32(CLK_CLKSTATUS)&HXT_READY) && millis()-t0<1500) { }
  Serial.print("12MHz external crystal: ");
  Serial.println((REG32(CLK_CLKSTATUS)&HXT_READY) ? "READY":"NOT READY");
  Serial.print("CLKSTATUS after = 0x");
  Serial.println(REG32(CLK_CLKSTATUS), HEX);
  Serial.println("Next stage: USB 48MHz clock + descriptors + endpoints + CDC ACM.");
}
void loop() {
  static uint32_t last=0;
  if(millis()-last>=1000) {
    last=millis();
    digitalWrite(LED,digitalRead(LED)==HIGH?LOW:HIGH);
    Serial.print("alive millis=");
    Serial.println(last);
  }
}
