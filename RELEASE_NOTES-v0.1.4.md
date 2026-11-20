# Arduino NUC140 v0.1.4 — Integrated I/O release

Unofficial, experimental Arduino Core for the **Nu-LB-NUC140 V2.0 (NUC140VE3CN, ARM Cortex-M0)**. This release integrates the functionality validated on one physical evaluation board, previously supplied as separate development patches.

## Confirmed on hardware (November 2026)

| Feature | Status | Example or observation |
| --- | --- | --- |
| GPIO / digitalWrite / digitalRead | PASS | Onboard PC12 LED, active LOW |
| Serial (UART0) | PASS | TX PB0, RX PB1, 115200 baud |
| ADC 12-bit analogRead | PASS | PA7 / VR1 0..4095 |
| Wire I2C1 | PASS | PA10 SDA / PA11 SCL, 24LC64 EEPROM at 0x50, 16-byte read |
| SPI2 / SPI.h | PASS | PD0 CS, PD1 SCK, PD2 MISO0, PD3 MOSI0, JEDEC EF 40 15 |
| NUC140LCD (SPI3 9-bit) | PASS | 128x64 LCD, hardware SS0 PD8, CLK PD9, MISO PD10, MOSI PD11, backlight PD14 |
| NUC140RTC (internal RTC) | PASS | RTC display on LCD and Serial time setting: RTC SET OK |
| analogWrite hardware PWM RGB | PASS | PA12 PWM0 blue, PA13 PWM1 green, PA14 PWM2 red; LED active LOW |

PWM v0.1.1 fixes an initial early return on an anomalous PWM_SYNC=0xFFFF read. At 50% duty on PA14: GPA_MFP=0x4000, PWM_PCR=0x90000, PWM_POE=0x4, CNR=0x1599, CMR=0x0AD7.

## Arduino usage

- GPIO: pinMode(PC(12), OUTPUT); digitalWrite(PC(12), LOW);
- UART: Serial.begin(115200); Serial.println("Hello");
- ADC: analogRead(PA(7));
- I2C: include Wire.h and Wire.begin();
- SPI flash: include SPI.h and SPI.begin();
- RGB PWM: analogWrite(PA(14), 128); for intuitive active-LOW RGB brightness, use rgbWrite(r,g,b) from NUC140PWM.h.
- LCD: include NUC140LCD.h; lcd.begin(); lcd.printText(0,0,"NUC140"); lcd.display();
- RTC: include NUC140RTC.h; RTCClock.begin(); RTCClock.getDateTime(dt).

## Install

Add this Additional Boards Manager URL in Arduino IDE:

https://raw.githubusercontent.com/subairi-git/Arduino-NUC140/main/package_nuc140_index.json

Then Boards Manager -> search **NUC140** -> install **0.1.4** -> select **Nu-LB-NUC140 V2.0 (NUC140VE3CN)**.

Previously installed manual patches are integrated into the 0.1.4 core. Do not overlay old v0.1.2 patches on top of v0.1.4.

Upload requires the Nuvoton **Nu-Link Command Tool** installed on Windows. It must identify NUC140VE3CN; flashing modifies the APROM application.

## Known limitations and test coverage

- This is a board-specific experimental core, not a general Arduino implementation for every NUC100/140 part.
- PWM v0.1.4 initial implementation supports PA12..PA15 at approximately 1 kHz with 8-bit values. RGB PWM0..PWM2 tested; PWM3 (PA15) not separately validated.
- SPI2 was tested reading a flash JEDEC ID. Write/erase, all SPI modes, and LSBFIRST have not been validated on hardware.
- I2C was tested with one on-board EEPROM and a short read; not every device or transfer mode.
- RTC reset persistence, backup battery across power loss, alarm interrupts and long-term time accuracy not yet validated.
- LCD library is specific to the board's 128x64 SPI3 9-bit panel, not all ST7565-like displays.
- Microcontroller timing assumes HCLK about 22.1184 MHz. Serial/ADC/RTC/PWM and their examples were tested individually; exhaustive simultaneous peripheral use was not tested.
- Installer / Boards Manager packaging and example build verification are distinct from physical peripheral testing.

## Source and docs

https://github.com/subairi-git/Arduino-NUC140
