#include <Arduino.h>
// NUC140 USB device VBUS detect diagnostic, NOT a USB CDC driver.
// USB Device connector J3 (not Nu-Link USB). USB pull-up is never enabled.
#define MMIO(x) (*(volatile uint32_t *)(x))
#define WRPROT 0x50000100UL
#define PWRCON 0x50000200UL
#define APBCLK 0x50000208UL
#define CLKSTATUS 0x5000020CUL
#define CLKSEL0 0x50000210UL
#define CLKDIV 0x50000218UL
#define PLLCON 0x50000220UL
#define USBD_ATTR 0x40060010UL
#define USBD_FLDET 0x40060014UL
#define USBD_INTSTS 0x40060004UL
#define USB_CLK_EN (1UL<<27)
#define PLL_READY (1UL<<2)
#define HXT_READY (1UL<<0)
static void hx(const char *label,uint32_t val) {
 Serial.print(label);Serial.print("=0x");Serial.println(val,HEX);
}
static void unlock() {MMIO(WRPROT)=0x59;MMIO(WRPROT)=0x16;MMIO(WRPROT)=0x88;}
void setup() {
 Serial.begin(115200);
 delay(400);
 Serial.println("NUC140 USB DEVICE VBUS DETECT TEST");
 uint32_t oldSel=MMIO(CLKSEL0);
 // Prepare separate 48 MHz clock for USB while leaving CPU source unchanged.
 noInterrupts();
 unlock();
 bool ok=(MMIO(WRPROT)&1)!=0;
 if(ok) MMIO(PWRCON)|=1UL;
 MMIO(WRPROT)=0;
 interrupts();
 if(!ok) {Serial.println("FAIL: register access");return;}
 uint32_t started=millis();
 while(!(MMIO(CLKSTATUS)&HXT_READY) && millis()-started<1500UL) {}
 if(!(MMIO(CLKSTATUS)&HXT_READY)) {
   Serial.println("FAIL: HXT 12MHz oscillator not ready");
   return;
 }
 // Activate USB PLL and APB module only after external crystal stabilizes.
 noInterrupts();
 unlock();
 ok=(MMIO(WRPROT)&1)!=0;
 if(ok) {
   MMIO(PLLCON)=0xC22EUL;
   MMIO(CLKDIV)&=~(15UL<<4); // USB clock divider /1
   MMIO(APBCLK)|=USB_CLK_EN;
 }
 MMIO(WRPROT)=0;
 interrupts();
 if(!ok) {Serial.println("FAIL: PLL register access");return;}
 started=millis();
 while(!(MMIO(CLKSTATUS)&PLL_READY) && millis()-started<1500UL) {}
 hx("CLKSTATUS ",MMIO(CLKSTATUS));
 hx("APBCLK    ",MMIO(APBCLK));
 hx("CLKDIV   ",MMIO(CLKDIV));
 hx("CLKSEL0  ",MMIO(CLKSEL0));
 if((MMIO(CLKSTATUS)&(HXT_READY|PLL_READY))!=(HXT_READY|PLL_READY)||
    MMIO(CLKSEL0)!=oldSel) {
   Serial.println("FAIL: USB clock/HCLK check");
   return;
 }
 Serial.println("PASS: USB module clock enabled (no D+ pull-up)");
 Serial.println("Connect/disconnect USB DEVICE J3, not Nu-Link.");
}
void loop(){
 static uint32_t last=0;
 if(millis()-last>=1000UL) {
   last=millis();
   Serial.print("millis=");Serial.print(last);
   Serial.print(" VBUS=");
   Serial.print((MMIO(USBD_FLDET)&1UL)?"ATTACHED":"DETACHED");
   Serial.print(" FLDET=0x");Serial.print(MMIO(USBD_FLDET),HEX);
   Serial.print(" ATTR=0x");Serial.print(MMIO(USBD_ATTR),HEX);
   Serial.print(" INTSTS=0x");Serial.println(MMIO(USBD_INTSTS),HEX);
 }
}
