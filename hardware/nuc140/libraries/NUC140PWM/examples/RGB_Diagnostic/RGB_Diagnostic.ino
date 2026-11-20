/*
 * Nu-LB-NUC140 V2.0: diagnose RGB GPIO first, then hardware PWM.
 * Works with the existing PWM v0.1.0 preview. Serial Monitor 115200.
 * PA14 RED, PA13 GREEN, PA12 BLUE; LEDs active LOW.
 */
#include <NUC140PWM.h>

#define REG32(a) (*(volatile uint32_t*)(a))
#define GPA_MFP   0x50000030UL
#define ALT_MFP   0x50000050UL
#define ALT_MFP1  0x50000058UL
#define APBCLK    0x50000208UL
#define CLKSEL1   0x50000214UL
#define CLKSEL2   0x5000021CUL
#define PWM_BASE  0x40040000UL

void hex32(const char* name, uint32_t x) {
  char buf[11]="0x00000000";
  for (int n=0; n<8; ++n) {
    uint8_t digit=(uint8_t)((x >> (28-4*n)) & 0xFU);
    buf[n+2]=(char)(digit<10 ? '0'+digit : 'A'+digit-10);
  }
  Serial.print(name); Serial.println(buf);
}
void dump(uint32_t pin) {
  uint32_t ch=pin-PA(12);
  Serial.println("--- PWM REGISTERS at analogWrite=128 ---");
  hex32("GPA_MFP=", REG32(GPA_MFP));
  hex32("ALT_MFP=", REG32(ALT_MFP));
  hex32("ALT_MFP1=",REG32(ALT_MFP1));
  hex32("APBCLK=",  REG32(APBCLK));
  hex32("CLKSEL1=", REG32(CLKSEL1));
  hex32("CLKSEL2=", REG32(CLKSEL2));
  hex32("PPR=",     REG32(PWM_BASE+0x00UL));
  hex32("CSR=",     REG32(PWM_BASE+0x04UL));
  hex32("PCR=",     REG32(PWM_BASE+0x08UL));
  hex32("POE=",     REG32(PWM_BASE+0x7CUL));
  hex32("CNR=",     REG32(PWM_BASE+0x0CUL+12UL*ch));
  hex32("CMR=",     REG32(PWM_BASE+0x10UL+12UL*ch));
  hex32("SYNC=",    REG32(PWM_BASE+0x88UL+4UL*ch));
}
void allOff() {
  analogWrite(PA(12),255);
  analogWrite(PA(13),255);
  analogWrite(PA(14),255);
}
void gpioTest(const char* name, uint32_t pin) {
  allOff();
  Serial.print("GPIO "); Serial.print(name); Serial.println(" ON");
  digitalWrite(pin,LOW);
  delay(1000);
  Serial.print("GPIO "); Serial.print(name); Serial.println(" OFF");
  digitalWrite(pin,HIGH);
  delay(1000);
}
void pwmTest(const char* name,uint32_t pin) {
  const int levels[]={0,64,128,192,255};
  allOff();
  Serial.print("PWM "); Serial.println(name);
  for(uint8_t n=0;n<5;++n) {
    analogWrite(pin,levels[n]);
    Serial.print(" analogWrite="); Serial.println(levels[n]);
    if(levels[n]==128) dump(pin);
    delay(1400);
  }
  allOff();
}
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("=== NUC140 RGB GPIO + PWM DIAGNOSTIC ===");
  Serial.println("PA14 RED; PA13 GREEN; PA12 BLUE; active LOW");
  allOff();
}
void loop() {
  Serial.println("PHASE A: GPIO ON/OFF");
  gpioTest("RED PA14",PA(14));
  gpioTest("GREEN PA13",PA(13));
  gpioTest("BLUE PA12",PA(12));
  Serial.println("PHASE B: PWM 0 64 128 192 255");
  pwmTest("RED PA14",PA(14));
  pwmTest("GREEN PA13",PA(13));
  pwmTest("BLUE PA12",PA(12));
  Serial.println("DONE, repeats in 4s");
  delay(4000);
}
