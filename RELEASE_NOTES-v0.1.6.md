# Arduino NUC140 v0.1.6 — Integrated I/O, 7-Segment and experimental USB CDC

Release for Nu-LB-NUC140 V2.0 (NUC140VE3CN), building on v0.1.5.

## New functionality
- `tone()` and `noTone()`: hardware Timer3-backed buzzer output on PB11, with Timer3 ownership protection.
- `SevenSegment4` library: 4-digit common-anode on-board display, validated glyph lookup, GPE0–7 data and GPC4–7 digit selectors. Automatic multiplexing via SysTick without allocating hardware timers.
- Experimental `SerialUSB` USB CDC ACM driver with descriptor handling, USB IRQ23, TX/RX buffers and echo example. `Serial` remains UART0.
- Integrated test example `Board_Integration_USB_7Seg_Timers_Buzzer`: validates simultaneous USB CDC, 7-segment, Timer0–2, and Timer3 buzzer.

## Hardware verification (26 November 2026)
On the real Nu-LB-NUC140 V2.0:
- USB CDC J3 TX and RX successful; echo `TEST123`.
- Timer0 1Hz, Timer1 10Hz, Timer2 20Hz concurrent; reported counters have exact 1:10:20 increments.
- Integrated 4-digit 7-segment display successful.
- Integrated PB11 buzzer using Timer3 `tone()` successful.
- USB CDC commands and responses successful in simultaneous integration test.

## USB VID/PID and release safety
**Important change from experimental development branch:** USB CDC now refuses to start until a valid **authorized** VID/PID pair is configured. Defaults are `NUC140_USB_VID=0` and `NUC140_USB_PID=0`; `SerialUSB.begin()` returns false by design in the public package if these remain unset. Edit `cores/arduino/USBSerialConfig.h` in your own local board installation or supply equivalent build flags with USB identifiers you are authorized to use. Never redistribute firmware with USB IDs not granted to you. The Nuvoton example VID/PID from development testing has been **removed from the shipping descriptor**.

## Status and limitations
This remains an **experimental community Arduino Core** for one target board. The integration test passed with development identifiers, while the public VID/PID opt-in safeguard has not been re-tested on hardware. Longer USB reconnect, hot-plug, power-loss, low-level compatibility and stress tests are not exhaustive. USB CDC is opt-in; Tone, timers, 7-segment, UART, ADC, PWM, I2C, SPI, RTC and LCD remain available independently.

## Install
Additional Boards Manager URL:
https://raw.githubusercontent.com/subairi-git/Arduino-NUC140/main/package_nuc140_index.json

Select NUC140 version 0.1.6 and the Nu-LB-NUC140 V2.0 board. Upload requires Nu-Link Command Tool on Windows. Existing versions remain available in the package index.

Source: https://github.com/subairi-git/Arduino-NUC140
