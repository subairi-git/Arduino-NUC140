#include "Arduino.h"

/*
 * NUC140VE3CN PA0..PE15 edge interrupts. GPIOA/B share IRQ4; GPIOC/D/E
 * (and GPIOF, not encoded by this experimental core) share IRQ5.
 *
 * Register definitions from OpenNuvoton/NUC100BSP:
 * GPIOx port size 0x40; IMD +0x18, IEN +0x1C, ISRC +0x20 (W1C).
 * IEN lower 16 enables falling edge, upper 16 enables rising edge.
 * IMD bit 0 = edge trigger. No GPIO multiplexing is changed here:
 * caller must configure GPIO input and avoid peripheral-owned pins.
 *
 * Callbacks must be short and ISR-safe (no Serial, LCD or delay).
 */
#define IRQ_REG32(addr) (*(volatile uint32_t *)(addr))
#define GPIO_BASE        0x50004000UL
#define GPIO_STEP        0x40UL
#define GPIO_IMD(port)   IRQ_REG32(GPIO_BASE + GPIO_STEP*(port) + 0x18UL)
#define GPIO_IEN(port)   IRQ_REG32(GPIO_BASE + GPIO_STEP*(port) + 0x1CUL)
#define GPIO_ISRC(port)  IRQ_REG32(GPIO_BASE + GPIO_STEP*(port) + 0x20UL)

#define NVIC_ISER        IRQ_REG32(0xE000E100UL)
#define NVIC_ICPR        IRQ_REG32(0xE000E280UL)

static NUC140InterruptCallback gpio_callbacks[80];

static uint32_t enter_critical(void) {
  uint32_t primask;
  __asm__ volatile("mrs %0, primask\ncpsid i"
                   : "=r"(primask) : : "memory");
  return primask;
}

static void exit_critical(uint32_t primask) {
  __asm__ volatile("msr primask, %0" : : "r"(primask) : "memory");
}

void attachInterrupt(uint32_t interruptNum, NUC140InterruptCallback callback, int mode) {
  if (interruptNum >= 80UL || callback == 0) return;
  if (mode != FALLING && mode != RISING && mode != CHANGE) return;

  const uint32_t port = interruptNum / 16UL;
  const uint32_t bit = 1UL << (interruptNum % 16UL);
  const uint32_t enabled = (bit | (bit << 16));
  const uint32_t irq = (port <= 1UL) ? 4UL : 5UL;

  uint32_t state = enter_critical();
  GPIO_IEN(port) &= ~enabled;
  GPIO_IMD(port) &= ~bit; // edge mode
  GPIO_ISRC(port) = bit;  // clear stale W1C flag before enabling
  gpio_callbacks[interruptNum] = callback;
  if (mode == FALLING || mode == CHANGE) GPIO_IEN(port) |= bit;
  if (mode == RISING  || mode == CHANGE) GPIO_IEN(port) |= (bit << 16);
  NVIC_ICPR = 1UL << irq;
  NVIC_ISER = 1UL << irq;
  exit_critical(state);
}

void detachInterrupt(uint32_t interruptNum) {
  if (interruptNum >= 80UL) return;
  const uint32_t port = interruptNum / 16UL;
  const uint32_t bit = 1UL << (interruptNum % 16UL);
  uint32_t state = enter_critical();
  GPIO_IEN(port) &= ~(bit | (bit << 16));
  GPIO_ISRC(port) = bit;
  gpio_callbacks[interruptNum] = 0;
  exit_critical(state);
}

static void service_gpio_ports(uint32_t first, uint32_t last) {
  for (uint32_t port = first; port <= last; port++) {
    const uint32_t flags = GPIO_ISRC(port) & 0xFFFFUL;
    if (!flags) continue;
    GPIO_ISRC(port) = flags; // W1C, BEFORE callbacks

    for (uint32_t bit = 0; bit < 16UL; bit++) {
      if ((flags & (1UL << bit)) != 0UL) {
        const uint32_t pin = port * 16UL + bit;
        NUC140InterruptCallback cb = gpio_callbacks[pin];
        if (cb != 0) cb();
      }
    }
  }
}

void GPAB_IRQHandler(void)   { service_gpio_ports(0UL, 1UL); }
void GPCDEF_IRQHandler(void) { service_gpio_ports(2UL, 4UL); }
