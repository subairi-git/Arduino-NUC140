# GPIO interrupt + TIMER0..TIMER3 preview (NUC140VE3CN)

**Status:** preview, ARM Cortex-M0 compilation only. Physical GPIO/timer interrupt
hardware tests are still required before merging or publishing Boards Manager
v0.1.5. Existing integrated v0.1.4 I/O release is unchanged.

## Hardware sources

The register and IRQ mapping follows Nuvoton's
[NUC100BSP](https://github.com/OpenNuvoton/NUC100BSP)
\`NUC100Series.h\`, \`gpio.c\` and \`timer.c\`.

| Feature | Hardware |
| --- | --- |
| GPIO PA/PB | shared external IRQ4, handler \`GPAB_IRQHandler\` |
| GPIO PC/PD/PE | shared external IRQ5, handler \`GPCDEF_IRQHandler\` |
| Timer 0 | IRQ8, APB1 0x40010000 |
| Timer 1 | IRQ9, APB1 0x40010020 |
| Timer 2 | IRQ10, APB2 0x40110000 |
| Timer 3 | IRQ11, APB2 0x40110020 |
| SW INT onboard | PB15, press generates FALLING edge (active LOW) |
| LED onboard | PC12, active LOW |
| LCD 128x64 | existing NUC140LCD / SPI3 PD8..PD11 |

## Standard Arduino-style GPIO interrupts

    pinMode(PB(15), INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PB(15)), onSwitch, FALLING);
    // Later:
    detachInterrupt(digitalPinToInterrupt(PB(15)));

Available modes: RISING, FALLING, CHANGE. Pin argument is the NUC140
PA/PB/PC/PD/PE encoding (0..79). Pins must already be configured as
**GPIO inputs**, not alternate peripheral functions. GPIO handlers scan
pending ISRC flags from their sharing ports and invoke registered callbacks.
The ISR clears source flags with write-one-to-clear before callback.

## Hardware timer API

    bool timerBegin(uint8_t index, uint32_t periodUs, void (*callback)(void));
    void timerStop(uint8_t index);
    void timerStart(uint8_t index);
    void timerEnd(uint8_t index);

\`timerBegin(0, 1000000, onSecond)\` starts **TIMER0** periodic interrupt
every 1,000,000 microseconds = about 1 second.
- Index 0..3 supported.
- Period must be 1..60,000,000 microseconds; compare >=2 and <=24 bits.
- 22.1184MHz internal HIRC used, configurable prescale 0..255.
- Period is quantized; not a guarantee of oscillator accuracy.
- Stops and restarts affect only the selected timer, not SysTick.
- Timer0..Timer3 share the system CLKSEL1 and APBCLK registers with other
  devices, so drivers use read-modify-write and preserve unrelated bits.
- No timer output/capture pin mux changes; periodic interrupt mode only.

## Examples

- \`Nuc140Examples > GPIO_SWINT_Interrupt\`: SW INT PB15 increments serial
  counter, toggles PC12 LED.
- \`Nuc140Examples > TIMER0_Blink\`: TIMER0 interrupts every 500ms, toggles PC12.
- \`NUC140LCD > LCD_Interrupt_Timer\`: TIMER0 elapsed seconds and PB15
  press count on the existing LCD, plus Serial monitor.

## Interrupt safety

The ISR functions should do minimal work, e.g. increment \`volatile uint32_t\`
variables or set flags. **Do not use Serial, LCD, SPI, Wire, delay, heap
allocation, or blocking operations inside an ISR.** Print and refresh the LCD
from \`loop()\` instead. The sketch includes a 180ms software press-time
filter based on millis; this reduces but does not guarantee suppression of
every mechanical bounce.

Do not use the same timer channel in two unrelated libraries.
GPIO pins in alternate-function mode (UART, PWM, ADC, SPI, I2C) should
not have GPIO interrupt callbacks attached unless switched to GPIO input.
RTC and SysTick are separate timer sources.

## How to install / restore v0.1.4

The optional ZIP installer overlays only
\`cores/arduino/{Arduino.h,startup.c,wiring_interrupts.c,wiring_timer.c}\`
and adds new example sketches. It keeps backups of the existing Arduino.h
and startup.c before changing them. It requires an installed Arduino Boards
Manager **v0.1.4** core and does not mutate the v0.1.4 GitHub Release.
A rollback script restores the original v0.1.4 core from those backups.

## Test checklist

1. GPIO SW INT PB15: press/release 1..10, expect 10 counts (if filter resolves bounces).
2. Timer0: PC12 LED blinks at ~500ms intervals even while the sketch does other work.
3. Combined LCD: elapsed second counter increments, SW INT count increases,
   Serial still works, screen does not freeze.
4. Retest previously verified Serial, ADC, Wire, SPI2, LCD, RTC, PWM if needed.
5. After results, update PR and only then consider integrated v0.1.5 release.
