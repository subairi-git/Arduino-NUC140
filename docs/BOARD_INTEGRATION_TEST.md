# NUC140 on-board integration test: USB CDC + 7-Segment + hardware timers + buzzer

Board: Nu-LB-NUC140 V2.0 (NUC140VE3CN).

Test intentionally runs these simultaneously:
- USB CDC ACM: connector J3, `SerialUSB` supports echo and simple commands.
- Built-in four-digit common-anode 7-segment: GPE0–7, GPC4–7, using automatic SysTick multiplex in the `SevenSegment4` library.
- Timer0 (1Hz), Timer1 (10Hz), Timer2 (20Hz) increment independent counters.
- Timer3 is **reserved for tone()** and not allocated to other timer tasks.
- Built-in buzzer PB11: a short tone every 5 seconds, 120ms duration.
- Existing UART0 (`Serial`) sends progress at 115200 baud through Nu-Link UART wiring (independent of USB CDC).

### Expected results

1. Display shows 1234 on setup, then starts showing 0000, 0001, 0002... once per second (Timer0 counter).
2. Buzzer PB11 sounds briefly every 5 seconds; if buzzer module active LOW, actual sound character may differ.
3. UART Serial Monitor prints `ms=... T0=... T1=... T2=... CDC=... DTR=... 7SEG=1 BEEP=1` every 2 seconds; at ~10 seconds, T0 ~10, T1 ~100, T2 ~200.
4. After USB CDC J3 appears as a Windows COM port, open that port. It prints `NUC140 OK T0=... T1=... T2=...` every 2 seconds when DTR is asserted. Send `TEST123`, which is echoed unless one of the characters is a command (`B`/`S`/`?`). For an unambiguous echo use `HELLO123`. Send `S` to silence the buzzer and `B` to resume, or `?` for status.
5. Monitor for display flicker, missing USB data, frozen timer counters, and tone disruptions. No false claim of validation until physical results are reported.

### Notes

Uses `SevenSegment4` library and revised `wiring.c` hook from `feature/usb-cdc-full`. The earlier single-display glyph-mapping sketch was hardware validated; this integrated version and automatic SysTick library still require hardware validation.

The USB CDC test driver uses Nuvoton example VID/PID 0416:B002 only for local development. Do not publish a production firmware with these identifiers without permission.

If USB CDC fails, UART still reports diagnostic counters; keep Nu-Link connection while testing. Preserve stable release v0.1.5.
