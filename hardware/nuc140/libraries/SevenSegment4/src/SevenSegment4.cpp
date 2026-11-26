#include <Arduino.h>
#include "SevenSegment4.h"

// Glyphs validated with the physical Nu-LB-NUC140 V2.0 board.
// Bit i drives GPEi (0 = LED segment ON), including DP bit.
static const uint8_t kDigitGlyph[10] = {
  0x82, 0xEE, 0x07, 0x46, 0x6A,
  0x52, 0x12, 0xE6, 0x02, 0x62
};
static const uint32_t kPcDigitMask = 0xF0UL;  // PC4...PC7
static const uint32_t kPeSegmentMask = 0xFFUL; // PE0...PE7
#define REG32(addr) (*reinterpret_cast<volatile uint32_t *>(addr))
static const uint32_t kPcDout = 0x50004088UL;
static const uint32_t kPeDout = 0x50004108UL;

static volatile uint8_t sGlyphs[4] = {0x82,0x82,0x82,0x82};
static volatile uint8_t sRunning = 0;
static uint8_t sDigit = 0;

static uint32_t saveInterruptState() {
  uint32_t primask;
  __asm__ volatile("mrs %0, primask\n cpsid i" : "=r"(primask) :: "memory");
  return primask;
}
static void restoreInterruptState(uint32_t primask) {
  __asm__ volatile("msr primask, %0" :: "r"(primask) : "memory");
}
// Called at 1kHz by the core's SysTick_Handler while the library is linked.
// Each position is refreshed at 250Hz. No delay() or serial inside an ISR.
extern "C" void sevenSegment4Tick(void) {
  if (!sRunning) return;
  // Prevent ghosting by blanking all digit drivers before changing the bus.
  REG32(kPcDout) &= ~kPcDigitMask;
  REG32(kPeDout) = (REG32(kPeDout) & ~kPeSegmentMask) | sGlyphs[sDigit];
  REG32(kPcDout) |= (1UL << (4U + sDigit));
  sDigit = (uint8_t)((sDigit + 1U) & 3U);
}
SevenSegment4Class SevenSegment4;
void SevenSegment4Class::begin() {
  // Ensure digit enables are OFF before changing pin directions.
  for (uint8_t i=0; i<4; ++i) {
    digitalWrite(PC(4U+i),LOW);
    pinMode(PC(4U+i),OUTPUT);
  }
  for (uint8_t i=0; i<8; ++i) {
    digitalWrite(PE(i),HIGH);
    pinMode(PE(i),OUTPUT);
  }
  const uint32_t state=saveInterruptState();
  REG32(kPcDout) &= ~kPcDigitMask;
  REG32(kPeDout) |= kPeSegmentMask;
  sDigit=0;
  sRunning=1;
  restoreInterruptState(state);
}
void SevenSegment4Class::end() {
  const uint32_t state=saveInterruptState();
  sRunning=0;
  REG32(kPcDout) &= ~kPcDigitMask;
  REG32(kPeDout) |= kPeSegmentMask;
  restoreInterruptState(state);
}
void SevenSegment4Class::display(uint16_t number) {
  uint8_t glyphs[4];
  number=(uint16_t)(number%10000U);
  for (uint8_t i=0;i<4;i++) {
    glyphs[i]=kDigitGlyph[number%10U]; // index 0 is the rightmost digit
    number=(uint16_t)(number/10U);
  }
  const uint32_t state=saveInterruptState();
  for(uint8_t i=0;i<4;i++) sGlyphs[i]=glyphs[i];
  restoreInterruptState(state);
}
void SevenSegment4Class::clear() {
  const uint32_t state=saveInterruptState();
  for(uint8_t i=0;i<4;i++) sGlyphs[i]=0xFFU;
  restoreInterruptState(state);
}
bool SevenSegment4Class::isRunning() const {return sRunning!=0;}
