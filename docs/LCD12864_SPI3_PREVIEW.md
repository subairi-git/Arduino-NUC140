# NUC140LCD — LCD 128x64 SPI3 9-bit preview

This preview targets the **Nu-LB-NUC140 V2.0** onboard monochrome 128x64 LCD.
The official board user manual (Rev 2.00, p.13) confirms PD8/PD9/PD11 for
LCD SPI3 and PD14 for active-low backlight/power. Educational NUC140 LCD
source demonstrates 9-bit SPI3: bit 8 = data (1), command (0), with an
ST7565-like page/column addressing scheme.

**Status:** cross-compilation succeeds, but real LCD hardware response
has not yet been reported. Exact LCD controller is not identified in the
board manual; initialization is an experimental ST7565-compatible candidate.
A SPI3 send operation can succeed without proving that text appeared.

## Install on the user's already-patched Board Manager v0.1.2

Use `NUC140-LCD12864-SPI3-Library-Install.zip`, extract then run
`INSTALL-LCD.cmd` with Arduino IDE closed.
Only `hardware/nuc140/libraries/NUC140LCD` is copied to the installed
board package: `%LOCALAPPDATA%\Arduino15\packages\nuc140\hardware\nuc140\0.1.2\libraries\NUC140LCD`.
The installer does **not** change UART0, ADC, Wire, SPI2, Nu-Link.

## Quick example

```cpp
#include <NUC140LCD.h>
NUC140LCD lcd;
void setup() {
  Serial.begin(115200);
  if (!lcd.begin()) Serial.println("LCD begin failed");
  lcd.clear();
  lcd.printText(0,0,"HELLO NUC140");
  lcd.display();
}
void loop() {}
```

- `File > Examples > NUC140LCD > LCD_Hello` does a 700ms all-pixel test
  and then draws a border and several lines of text.
- `LCD_VR1_Gauge` displays the built-in analogRead(PA(7)) value as a bar.

## Driver

Independent SPI3 9-bit synchronous master (no dependency on the SPI2
Flash library). SPI3 uses a software chip-select on PD8, SPI3 CLK on PD9
and MOSI0 on PD11; reads are not required. PD14 backlight active low.
Source clock assumed HCLK = 22.1184 MHz; SPI3 configured to ~1 MHz.
1 KB framebuffer; original 5x7 ASCII subset font, lower-case displayed
as upper-case. `display()` sends all 8 pages.

Available API: `begin`, `clear`, `fill`, `display`, `pixel`,
`line`, `rect`, `fillRect`, `drawChar`, `printText`,
`printLine`, `backlight`, `setContrast`, `lastError`.

If backlight lights but text does not appear, verify the exact panel
controller, contrast level, 9-bit framing, chip select, and column mapping.
`lastError=0` indicates transfers completed without an SPI timeout,
NOT proof of visible pixels.

References:
- https://www.nuvoton.com/resource-files/UM_Nu-LB-NUC140_EN_Rev2.0.pdf
- https://github.com/OpenNuvoton/NUC100BSP
