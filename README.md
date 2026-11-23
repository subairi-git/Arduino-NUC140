# Arduino NUC140 V2.0 — v0.1.5 experimental community core

Unofficial Arduino-style platform for **Nu-LB-NUC140 V2.0 (NUC140VE3CN, ARM Cortex-M0)**, programmed with a **Nu-Link** probe on Windows. This release combines the tested core and peripheral libraries in a single Boards Manager installation.

## Install with Arduino Boards Manager

**Additional Boards Manager URL** (add to File > Preferences > Additional Boards Manager URLs):

https://raw.githubusercontent.com/subairi-git/Arduino-NUC140/main/package_nuc140_index.json

Then use **Tools > Board > Boards Manager**, search **NUC140**, and install **v0.1.5**. Select **Nu-LB-NUC140 V2.0 (NUC140VE3CN)** before verifying/uploading. The official Nuvoton **Nu-Link Command Tool** must be installed separately on Windows; it is not bundled. Do not copy the older manual v0.1.2 library patches over the integrated v0.1.5 package.

## Integrated APIs

| Arduino API / Library | Board-specific mapping | Physical test |
| --- | --- | --- |
| pinMode, digitalWrite, digitalRead | PA/PB/PC/PD/PE macros; onboard PC12 LED active LOW | PASS |
| Serial (UART0) | PB0 TX, PB1 RX, 115200 baud | PASS |
| analogRead | PA0..PA7 12-bit, VR1 on PA7 | PASS on PA7 |
| Wire | I2C1: PA10 SDA, PA11 SCL; EEPROM 0x50 | PASS |
| SPI | SPI2: PD0 CS, PD1 CLK, PD2 MISO0, PD3 MOSI0 | PASS, JEDEC read |
| NUC140LCD | SPI3 9-bit, PD8–PD11, PD14 backlight; 128x64 | PASS |
| NUC140RTC | Internal RTC with board 32.768-kHz crystal | PASS for read/set |
| analogWrite / NUC140PWM | PA12 PWM0 blue, PA13 PWM1 green, PA14 PWM2 red | PASS for RGB |

Example sketches ship under their relevant library folders (for example, NUC140PWM > RGB_Fade, NUC140RTC > RTC_Serial_Clock, NUC140LCD > LCD_Hello).

### Example: standard Arduino PWM

```cpp
void setup() {}
void loop() {
  for (int value=0; value<=255; value+=5) {
    analogWrite(PA(14), value); // Red LED active LOW: lower means brighter
    delay(20);
  }
}
```

For intuitive 0=off, 255=brightest RGB colors, use `#include <NUC140PWM.h>` and `rgbWrite(r,g,b)`.

## GPIO interrupt and hardware timers

GPIO: \`attachInterrupt(digitalPinToInterrupt(PB(15)), callback, FALLING)\`, \`detachInterrupt(PB(15))\`. Valid modes: \`RISING\`, \`FALLING\`, \`CHANGE\`. GPIO PA..PE are supported in the core; PB14/PB15 use EINT0/EINT1. Callbacks run in an ISR, so avoid \`delay()\` or blocking serial I/O within callbacks.

Timers: \`timerBegin(index, periodMicroseconds, callback)\` configures and starts a periodic timer, index 0..3. Other functions: \`timerStart\`, \`timerStop\`, \`timerSetPeriod\`, \`timerEnd\`, \`timerAttachInterrupt\`, \`timerDetachInterrupt\`.

Verified on one board: PB15, four concurrent timers and Timer0 lifecycle. Other GPIO modes/pins and extended soak tests remain experimental.

## Programming via Nu-Link

Uploading replaces the current **APROM program**, not all MCU memory. The Windows uploader validates the firmware and checks the connected device identity via `NuLink.exe -p` for **NUC140VE3CN** before erasing/programming/verifying APROM. Keep only one Nu-Link attached. The tool does not modify LDROM, DataFlash or configuration bits by design.

If NuLink.exe cannot be located, set the user environment variable `NUC140_NULINK_EXE` to its full path (often `C:\Program Files (x86)\Nuvoton Tools\NuLink Command Tool\NuLink.exe`) and restart Arduino IDE.

## Important scope notes

This is **experimental**. The physical tests described above were conducted on one Nu-LB-NUC140 V2.0 board, not across all pin mappings, chips or Arduino libraries. PWM3/PA15, SPI flash write/erase, complex concurrent I/O, and RTC backup operation after complete power loss have not been confirmed. Timing assumes HCLK 22.1184 MHz. For further details, see [Release notes v0.1.5](RELEASE_NOTES-v0.1.5.md) and [PWM notes](docs/PWM_RGB_PREVIEW.md).

## Sources and license

- [NUC100 BSP / standard drivers](https://github.com/OpenNuvoton/NUC100BSP)
- [Nu-LB-NUC140 board user manual](https://www.nuvoton.com/resource-files/UM_Nu-LB-NUC140_EN_Rev2.0.pdf)
- [Arduino platform specification](https://docs.arduino.cc/arduino-cli/platform-specification/)

New core glue code is under MIT license. Proprietary Nuvoton NuLink.exe and drivers are installed independently.
