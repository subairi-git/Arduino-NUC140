# Arduino NUC140 v0.1.5 — GPIO Interrupt and Hardware Timers

This experimental release extends v0.1.4 while preserving the existing UART, LCD, PWM, ADC, RTC, Wire and SPI support.

## Added
- `attachInterrupt(digitalPinToInterrupt(pin), callback, RISING|FALLING|CHANGE)` and `detachInterrupt(pin)` for PA..PE (PB14=EINT0, PB15=EINT1).
- Periodic hardware Timer0..Timer3, microsecond API: `timerBegin`, `timerStart`, `timerStop`, `timerEnd`, `timerSetPeriod`, `timerAttachInterrupt`, `timerDetachInterrupt`.
- Examples: GPIO_Interrupt_PB15, Timer0_3_Demo and Timer_Lifecycle_Test.

## Hardware testing
- PB15 falling-edge callback produced sequential button press counts.
- All four timers simultaneously produced expected 1 Hz, 2 Hz, 4 Hz, 10 Hz counts.
- Timer0 lifecycle 2 Hz -> stop -> restart -> 4 Hz -> end verified on one board.
- Other pins/modes, long-term clock drift, and extreme interrupt load have not been exhaustively tested.

**Board:** Nu-LB-NUC140 V2.0 (NUC140VE3CN). Core assumes HIRC 22.1184 MHz; this is an experimental community core.
