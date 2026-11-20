#include "Arduino.h"

/*
 * NUC140VE3CN hardware PWM output: PA12/PWM0 through PA15/PWM3.
 * PWMA HCLK = 22,118,400 Hz (matching the experimental core's SysTick).
 * PWM source = HCLK; prescaler CP=1 => /2, CSR=0 => /2,
 * period 5530 counts => 999.9277 Hz. 8-bit Arduino analogWrite range.
 * Register definitions cross-checked against OpenNuvoton NUC100BSP.
 * This initial release deliberately leaves PWM4..7 out of scope.
 *
 * PWM0/1 share a prescaler, PWM2/3 share a prescaler.
 * A fixed frequency avoids accidental pairwise changes.
 */
#define PWM_REG32(a) (*(volatile uint32_t *)(a))
#define PWM_PROTECT    0x50000100UL
#define PWM_GPA_MFP    0x50000030UL
#define PWM_ALT_MFP    0x50000050UL
#define PWM_APBCLK     0x50000208UL
#define PWM_CLKSEL1    0x50000214UL
#define PWM_CLKSEL2    0x5000021CUL
#define PWM_A_BASE     0x40040000UL
#define PWM_PPR        (PWM_A_BASE + 0x00UL)
#define PWM_CSR        (PWM_A_BASE + 0x04UL)
#define PWM_PCR        (PWM_A_BASE + 0x08UL)
#define PWM_POE        (PWM_A_BASE + 0x7CUL)
#define PWM_CNR(ch)    (PWM_A_BASE + 0x0CUL + 12UL*(ch))
#define PWM_CMR(ch)    (PWM_A_BASE + 0x10UL + 12UL*(ch))
#define PWM_SYNC(ch)   (PWM_A_BASE + 0x88UL + 4UL*(ch))
#define PWM_PERIOD_TICKS 5530UL
#define PWM_WAIT_LIMIT 100000UL

static uint8_t pwm_initialized;
static uint8_t pwm_channel_initialized[4];

static int pwm_wait(uint32_t ch) {
  uint32_t timeout = PWM_WAIT_LIMIT;
  while ((PWM_REG32(PWM_SYNC(ch)) & 1UL) && timeout) --timeout;
  return timeout != 0UL;
}

static int pwm_unlock(void) {
  PWM_REG32(PWM_PROTECT)=0x59UL;
  PWM_REG32(PWM_PROTECT)=0x16UL;
  PWM_REG32(PWM_PROTECT)=0x88UL;
  return (PWM_REG32(PWM_PROTECT)&1UL)!=0UL;
}

static void pwm_lock(void) { PWM_REG32(PWM_PROTECT)=0UL; }

static int pwm_init(void) {
  if (pwm_initialized) return 1;
  if (!pwm_unlock()) return 0;

  PWM_REG32(PWM_APBCLK) |= (1UL<<20) | (1UL<<21); // PWM01, PWM23 clocks
  PWM_REG32(PWM_CLKSEL1) =
    (PWM_REG32(PWM_CLKSEL1) & ~(15UL<<28)) | (2UL<<28) | (2UL<<30);
  PWM_REG32(PWM_CLKSEL2) &= ~((1UL<<8) | (1UL<<9)); // HCLK, not low RC
  pwm_lock();

  // PWM prescaler CP01 / CP23 = 1 => divide by 2; CSR divide 2.
  // Does not alter unrelated dead-zone bits in high half of PPR.
  PWM_REG32(PWM_PPR) = (PWM_REG32(PWM_PPR) & ~0xFFFFUL) | 0x0101UL;
  PWM_REG32(PWM_CSR) &= ~0x7777UL;
  for (uint32_t ch=0; ch<4UL; ch++) {
    if (!pwm_wait(ch)) return 0;
  }
  pwm_initialized=1U;
  return 1;
}

static void pwm_pin_mux(uint32_t channel, int enable) {
  const uint32_t mask=1UL << (12UL+channel);
  if (!pwm_unlock()) return;
  if (enable) {
    // PA12..PA14 may conflict with EBI; PA15 may conflict with I2S MCLK.
    const uint32_t extraMask = channel==0UL ? (1UL<<21) :
                               channel==1UL ? (1UL<<22) :
                               channel==2UL ? (1UL<<23) : (1UL<<9);
    PWM_REG32(PWM_ALT_MFP) &= ~((1UL<<11) | extraMask);
    PWM_REG32(PWM_GPA_MFP) |= mask;
  } else {
    PWM_REG32(PWM_GPA_MFP) &= ~mask;
  }
  pwm_lock();
}

void analogWrite(uint32_t pin, int value) {
  // This preview only supports 4 hardware PWM outputs PA12..PA15.
  if (pin < PA(12) || pin > PA(15)) return;
  const uint32_t ch = pin - PA(12);
  if (value < 0) value=0;
  if (value > 255) value=255;
  const uint32_t mask=1UL << ch;
  const uint32_t shift=8UL*ch;

  if (value == 0 || value == 255) {
    // Exact 0% / 100% voltage output: avoid CMR=0 one-clock high pulse.
    // Stage GPIO state before switching pin multiplexer away from PWM.
    digitalWrite(pin, value == 255 ? HIGH : LOW);
    pinMode(pin,OUTPUT);
    if (pwm_initialized) {
      PWM_REG32(PWM_POE) &= ~mask;
      PWM_REG32(PWM_PCR) &= ~(1UL<<shift);  // stop this channel only
    }
    pwm_pin_mux(ch,0);
    return;
  }

  if (!pwm_init()) return;
  if (!pwm_channel_initialized[ch]) {
    // Edge-aligned, auto-reload, non-inverted output.
    PWM_REG32(PWM_PCR) = (PWM_REG32(PWM_PCR) &
      ~((7UL<<(shift+1)) | (1UL<<(30UL + (ch/2UL))))) |
      (1UL<<(shift+3UL));
    if (!pwm_wait(ch)) return;
    PWM_REG32(PWM_CNR(ch))=PWM_PERIOD_TICKS - 1UL;
    if (!pwm_wait(ch)) return;
    pwm_channel_initialized[ch]=1U;
  }
  const uint32_t ticks = ((uint32_t)value * PWM_PERIOD_TICKS + 127UL)/255UL;
  PWM_REG32(PWM_CMR(ch))=ticks-1UL;
  if (!pwm_wait(ch)) return;
  PWM_REG32(PWM_POE) |= mask;
  PWM_REG32(PWM_PCR) |= (1UL<<shift);
  pwm_pin_mux(ch,1);
}
