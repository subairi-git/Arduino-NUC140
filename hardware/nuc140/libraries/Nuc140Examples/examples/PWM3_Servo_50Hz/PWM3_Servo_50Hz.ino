#include <Arduino.h>
/*
 * Nu-LB-NUC140 V2.0 (NUC140VE3CN): RC servo using TRUE hardware PWM3.
 *
 * Signal: PA15 / PWM3. Servo power from suitable EXTERNAL supply;
 * connect supply GND to NUC140 GND. GPIO cannot power a servo.
 * This standalone example configures PWM3 at 50Hz (20ms period),
 * and outputs 1000..2000us pulses, initially CENTERED at 1500us.
 *
 * IMPORTANT: PWM2 (PA14) shares the PWM2/3 prescaler with PWM3.
 * Do not call analogWrite(PA(14),...) or analogWrite(PA(15),...)
 * while this example owns PWM3. analogWrite() assumes 1kHz.
 * This example is experimental and still needs physical servo testing.
 */
#define REG32(a) (*(volatile uint32_t *)(a))
static const uint32_t PROTECT=0x50000100UL;
static const uint32_t GPA_MFP=0x50000030UL;
static const uint32_t ALT_MFP=0x50000050UL;
static const uint32_t APBCLK=0x50000208UL;
static const uint32_t CLKSEL1=0x50000214UL;
static const uint32_t CLKSEL2=0x5000021CUL;
static const uint32_t PWM_BASE=0x40040000UL;
static const uint32_t PPR=PWM_BASE+0x00UL;
static const uint32_t CSR=PWM_BASE+0x04UL;
static const uint32_t PCR=PWM_BASE+0x08UL;
static const uint32_t CNR3=PWM_BASE+0x0CUL+12UL*3UL;
static const uint32_t CMR3=PWM_BASE+0x10UL+12UL*3UL;
static const uint32_t POE=PWM_BASE+0x7CUL;

// Core HCLK: 22,118,400 Hz, PWM3 source HCLK,
// PPR CP23=7 => /8, CSR PWM3=0 => /2, tick=1,382,400Hz.
// 50 Hz frame => 27648 ticks, within 16-bit CNR register range.
static const uint32_t PERIOD_TICKS=27648UL;
static bool servoAttached=false;
static bool unlockRegisters() {
  REG32(PROTECT)=0x59UL;REG32(PROTECT)=0x16UL;REG32(PROTECT)=0x88UL;
  return (REG32(PROTECT)&1UL)!=0;
}
static void lockRegisters() {REG32(PROTECT)=0UL;}

static void servoWriteMicroseconds(uint16_t us) {
  if(!servoAttached)return;
  if(us<1000U)us=1000U;
  if(us>2000U)us=2000U;
  // Width in PWM input ticks, with integer rounding.
  const uint32_t ticks=((uint32_t)us*13824UL+5000UL)/10000UL;
  REG32(CMR3)=ticks-1UL;
}
static bool servoBegin() {
  // PWM3 on PA15. Start in a neutral, stationary position.
  pinMode(PA(15),OUTPUT);
  digitalWrite(PA(15),LOW);
  if(!unlockRegisters())return false;
  REG32(APBCLK)|=(1UL<<21); // PWM2/3 module clock
  REG32(CLKSEL1)=(REG32(CLKSEL1)&~(3UL<<30))|(2UL<<30);
  REG32(CLKSEL2)&=~(1UL<<9); // PWM3 clock = HCLK
  // PA15 PWM3 function; clear conflicting I2S MCLK option.
  REG32(ALT_MFP)&=~((1UL<<11)|(1UL<<9));
  // Avoid accidental pulse before PERIOD/CMR setup.
  REG32(POE)&=~(1UL<<3);
  REG32(PCR)&=~(1UL<<24);
  // PWM2/3 shared prescaler is intentionally set for 50Hz PWM3.
  REG32(PPR)=(REG32(PPR)&~0xFF00UL)|0x0700UL;
  REG32(CSR)&=~(7UL<<12); // PWM3 clock divider /2
  REG32(PCR)=(REG32(PCR)&~((7UL<<25)|(1UL<<31)))|(1UL<<27);
  REG32(CNR3)=PERIOD_TICKS-1UL;
  REG32(CMR3)=2074UL; // 1500us = 2073.6 ticks, index 0-based
  REG32(GPA_MFP)|=(1UL<<15);
  REG32(POE)|=(1UL<<3);
  REG32(PCR)|=(1UL<<24);
  lockRegisters();
  servoAttached=true;
  return true;
}
static void servoEnd() {
  REG32(PCR)&=~(1UL<<24);
  REG32(POE)&=~(1UL<<3);
  if(unlockRegisters()){
    REG32(GPA_MFP)&=~(1UL<<15);
    lockRegisters();
  }
  pinMode(PA(15),OUTPUT);
  digitalWrite(PA(15),LOW);
  servoAttached=false;
}
static void servoWriteAngle(uint8_t angle) {
  if(angle>180U)angle=180U;
  servoWriteMicroseconds((uint16_t)(1000UL+((uint32_t)angle*1000UL+90UL)/180UL));
}

void setup() {
  Serial.begin(115200);
  if(!servoBegin()){
    Serial.println("PWM3 SERVO INIT FAIL");
    return;
  }
  Serial.println("NUC140 PWM3 SERVO READY: PA15, 50Hz, 1500us center");
  Serial.println("Moves 90 -> 45 -> 90 -> 135 -> 90 every 2 seconds");
  servoWriteAngle(90);
}
void loop() {
  if(!servoAttached)return;
  static const uint8_t angleSteps[]={90,45,90,135,90};
  static uint8_t index=0;
  static uint32_t last=millis();
  if((uint32_t)(millis()-last)>=2000UL) {
    last=millis();
    index=(uint8_t)((index+1U)%5U);
    servoWriteAngle(angleSteps[index]);
    Serial.print("Servo angle=");
    Serial.println((int)angleSteps[index]);
  }
}
