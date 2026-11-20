#include "Arduino.h"

/*
 * Hardware TIMER0..TIMER3 periodic interrupts, 22.1184 MHz HIRC.
 * NUC100BSP register map: TIMER0/1 on APB1 0x40010000/0x40010020,
 * TIMER2/3 on APB2 0x40110000/0x40110020.
 * TCSR +0x00, TCMPR +0x04 (24-bit compare, must be >= 2),
 * TISR +0x08 (W1C bit0), TEXCON +0x14.
 *
 * CLKSEL1 TMRn_S at 8 + 4*n, setting 7 chooses HIRC.
 * CLK APBCLK timer enable bit 2+n; NVIC IRQn = 8+n.
 * Timer callbacks run in interrupt context and must be short.
 */
#define TIMER_REG32(addr) (*(volatile uint32_t *)(addr))
#define REG_PROTECT 0x50000100UL
#define REG_PWRCON 0x50000200UL
#define REG_APBCLK 0x50000208UL
#define REG_CLKSTATUS 0x5000020CUL
#define REG_CLKSEL1 0x50000214UL
#define NVIC_ISER  0xE000E100UL
#define NVIC_ICER  0xE000E180UL
#define NVIC_ICPR  0xE000E280UL
#define TIMER_HIRC_HZ 22118400ULL
#define TIMER_MIN_COMPARE 2ULL
#define TIMER_MAX_COMPARE 0xFFFFFFULL
#define TIMER_MAX_PERIOD_US 60000000UL
#define TIMER_WAIT_LIMIT 1000000UL
#define TCSR_CEN (1UL<<30)
#define TCSR_IE  (1UL<<29)
#define TCSR_PERIODIC (1UL<<27)

static NUC140InterruptCallback timer_callbacks[4];
static uint8_t timer_prepared[4];

static volatile uint32_t *const timer_registers[4] = {
  (volatile uint32_t *)0x40010000UL,
  (volatile uint32_t *)0x40010020UL,
  (volatile uint32_t *)0x40110000UL,
  (volatile uint32_t *)0x40110020UL
};

static uint32_t save_interrupt_state(void) {
  uint32_t previous;
  __asm__ volatile("mrs %0, primask\ncpsid i"
                   : "=r"(previous) : : "memory");
  return previous;
}
static void restore_interrupt_state(uint32_t previous) {
  __asm__ volatile("msr primask, %0" : : "r"(previous) : "memory");
}

static int unlock_sys(void) {
  TIMER_REG32(REG_PROTECT) = 0x59UL;
  TIMER_REG32(REG_PROTECT) = 0x16UL;
  TIMER_REG32(REG_PROTECT) = 0x88UL;
  return (TIMER_REG32(REG_PROTECT) & 1UL) != 0UL;
}
static void lock_sys(void) { TIMER_REG32(REG_PROTECT) = 0UL; }

bool timerBegin(uint8_t timerIndex, uint32_t periodUs, NUC140InterruptCallback callback) {
  if (timerIndex > 3U || callback == 0 ||
      periodUs == 0UL || periodUs > TIMER_MAX_PERIOD_US) return false;

  /*
   * Compute period in HIRC clock cycles, then choose the smallest prescaler
   * so the 24-bit compare value is valid. Rounded to nearest count.
   */
  const uint64_t desired =
      (TIMER_HIRC_HZ * (uint64_t)periodUs + 500000ULL) / 1000000ULL;
  uint32_t prescale = 0, compare = 0;
  for (uint32_t divisor=1UL; divisor<=256UL; divisor++) {
    const uint64_t count = (desired + divisor/2UL) / divisor;
    if (count >= TIMER_MIN_COMPARE && count <= TIMER_MAX_COMPARE) {
      prescale = divisor - 1UL;
      compare = (uint32_t)count;
      break;
    }
  }
  if (compare == 0UL) return false;

  const uint32_t state = save_interrupt_state();
  const uint32_t irqMask = 1UL << (8U + timerIndex);
  volatile uint32_t *t = timer_registers[timerIndex];

  TIMER_REG32(NVIC_ICER) = irqMask;
  t[0] = 0UL;                // stop timer before changing clock
  t[5] = 0UL;                // disable optional capture
  t[2] = 1UL;                // clear stale timeout flag (W1C)
  timer_callbacks[timerIndex] = 0;
  timer_prepared[timerIndex] = 0U;

  if (!unlock_sys()) {
    restore_interrupt_state(state);
    return false;
  }

  TIMER_REG32(REG_PWRCON) |= (1UL << 2); // enable HIRC
  uint32_t remaining = TIMER_WAIT_LIMIT;
  while ((TIMER_REG32(REG_CLKSTATUS) & (1UL << 4)) == 0UL && remaining) {
    remaining--;
  }
  if (!remaining) {
    lock_sys();
    restore_interrupt_state(state);
    return false;
  }

  // Preserve UART, ADC, PWM, SPI and other timer clock selections.
  const uint32_t shift = 8UL + 4UL*(uint32_t)timerIndex;
  TIMER_REG32(REG_CLKSEL1) =
      (TIMER_REG32(REG_CLKSEL1) & ~(7UL << shift)) | (7UL << shift);
  TIMER_REG32(REG_APBCLK) |= 1UL << (2UL + timerIndex);
  lock_sys();

  t[0] = TCSR_PERIODIC | prescale; // configure, not running
  t[1] = compare;                 // counter compare
  t[2] = 1UL;                   // clear timeout before enabling IRQ
  timer_callbacks[timerIndex] = callback;
  timer_prepared[timerIndex] = 1U;
  TIMER_REG32(NVIC_ICPR) = irqMask;
  TIMER_REG32(NVIC_ISER) = irqMask;
  t[0] = TCSR_PERIODIC | prescale | TCSR_IE | TCSR_CEN;
  restore_interrupt_state(state);
  return true;
}

void timerStop(uint8_t timerIndex) {
  if (timerIndex > 3U) return;
  const uint32_t state = save_interrupt_state();
  volatile uint32_t *t = timer_registers[timerIndex];
  t[0] &= ~TCSR_CEN;
  t[2] = 1UL;
  TIMER_REG32(NVIC_ICPR) = 1UL << (8U + timerIndex);
  restore_interrupt_state(state);
}

void timerStart(uint8_t timerIndex) {
  if (timerIndex > 3U || !timer_prepared[timerIndex]) return;
  const uint32_t state = save_interrupt_state();
  volatile uint32_t *t = timer_registers[timerIndex];
  t[2] = 1UL;
  TIMER_REG32(NVIC_ICPR) = 1UL << (8U + timerIndex);
  t[0] |= TCSR_IE | TCSR_CEN;
  restore_interrupt_state(state);
}

void timerEnd(uint8_t timerIndex) {
  if (timerIndex > 3U) return;
  const uint32_t state = save_interrupt_state();
  const uint32_t mask = 1UL << (8U + timerIndex);
  TIMER_REG32(NVIC_ICER) = mask;
  volatile uint32_t *t = timer_registers[timerIndex];
  t[0] = 0UL;
  t[2] = 1UL;
  timer_callbacks[timerIndex] = 0;
  timer_prepared[timerIndex] = 0U;
  TIMER_REG32(NVIC_ICPR) = mask;
  restore_interrupt_state(state);
}

static void timer_irq(uint8_t index) {
  volatile uint32_t *t = timer_registers[index];
  if ((t[2] & 1UL) != 0UL) {
    t[2] = 1UL;  // W1C before callback
    NUC140InterruptCallback cb = timer_callbacks[index];
    if (cb) cb();
  }
}

void TMR0_IRQHandler(void) { timer_irq(0U); }
void TMR1_IRQHandler(void) { timer_irq(1U); }
void TMR2_IRQHandler(void) { timer_irq(2U); }
void TMR3_IRQHandler(void) { timer_irq(3U); }
