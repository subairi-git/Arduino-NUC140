#include <Arduino.h>
/*
  Nu-LB-NUC140 V2.0 board-mounted 4-digit common-anode seven segment.
  GPE0..GPE7: shared segment bus (LOW = ON).
  GPC4..GPC7: digit control (HIGH = ON).
  PC4 corresponds to the RIGHTMOST digit (ones), PC7 to leftmost.
  Segment encoding follows a tested NUC140 example:
  https://gist.github.com/mavif/d31d160ec7bebdfbe4e5

  This is a hardware validation sketch with no dependency on USB CDC.
*/
static const uint8_t SEG_DIGITS[10] = {
  0x82, 0xEE, 0x07, 0x46, 0x6A,
  0x52, 0x12, 0xE6, 0x02, 0x62
};
static uint8_t digits[4] = {4,3,2,1}; // index 0 = rightmost, shows 1234
static uint8_t activeDigit=0;
static uint32_t nextRefresh=0, phaseStart=0;
static uint8_t phase=0;

static inline void allDigitsOff() {
  for(uint8_t d=0; d<4; d++) digitalWrite(PC(4+d), LOW);
}
static inline void segmentBits(uint8_t raw) {
  for(uint8_t bit=0; bit<8; bit++) {
    digitalWrite(PE(bit), (raw & (1U<<bit)) ? HIGH : LOW);
  }
}
static void refresh() {
  allDigitsOff(); // blank during update to avoid ghosting
  segmentBits(SEG_DIGITS[digits[activeDigit]]);
  digitalWrite(PC(4+activeDigit), HIGH); // one digit at a time
  activeDigit=(activeDigit+1U)&3U;
}
static void setNumber(uint16_t n) {
  for(uint8_t d=0; d<4; d++) {
    digits[d]=n%10U;
    n/=10U;
  }
}
void setup() {
  Serial.begin(115200);
  // Set segment outputs OFF before enabling digit drivers.
  for(uint8_t d=0; d<4; d++) {
    pinMode(PC(4+d), OUTPUT);
    digitalWrite(PC(4+d), LOW);
  }
  for(uint8_t b=0; b<8; b++) {
    pinMode(PE(b), OUTPUT);
    digitalWrite(PE(b), HIGH);
  }
  nextRefresh=phaseStart=millis();
  Serial.println("NUC140 4x7SEG COMMON ANODE - Nuvoton wiring LUT");
  Serial.println("PC4 rightmost / PC7 leftmost; GPE0..7 segment bus");
  Serial.println("Sequence: 1234 -> 8888 -> 0000 -> counter");
}
void loop() {
  uint32_t now=millis();
  if((uint32_t)(now-nextRefresh)>=2U) {
    nextRefresh=now;
    refresh();
  }
  if((uint32_t)(now-phaseStart)>=3000UL) {
    phaseStart=now;
    phase=(phase+1U)&3U;
    Serial.print("7SEG phase=");Serial.println(phase);
  }
  switch(phase) {
    case 0:setNumber(1234);break;
    case 1:setNumber(8888);break;
    case 2:setNumber(0);break;
    default:setNumber((uint16_t)((now/100UL)%10000UL));break;
  }
}
