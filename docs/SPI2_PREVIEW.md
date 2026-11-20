# SPI2 Arduino SPI.h preview for Nu-LB-NUC140 V2.0

**Status: preview, not yet hardware-verified**. The SPI2 master library and
read-only W25X16 Flash examples cross-compile for ARM Cortex-M0.
UART0 Serial, analogRead(), and I2C1 Wire.h have already been tested on
the real NUC140VE3CN board. No existing core or library files are changed
by installing this preview SPI library.

## Wiring (onboard W25X16, lower U3)

| Signal | Port | Arduino macro |
|---|---|---|
| CS / SS | PD0 GPIO (software chip-select) | PD(0) |
| SCK | PD1 SPI2_CLK | PD(1) |
| MISO | PD2 SPI2_MISO0 | PD(2) |
| MOSI | PD3 SPI2_MOSI0 | PD(3) |

From *Nu-LB-NUC140 User Manual Rev 2.00*, page 19 of 34.
The board also shows an upper SPI Flash chip using PD4/PD5 data pins
with shared PD0 CS and PD1 SCK. This preview targets only **lower U3**
using PD2/PD3. It does not access PD4/PD5.

## API

- SPI.begin(), SPI.end()
- SPISettings(clockHz, MSBFIRST/LSBFIRST, SPI_MODE0..3)
- SPI.beginTransaction(settings), SPI.endTransaction()
- SPI.transfer(byte), SPI.transfer16(word), SPI.transfer(buffer, length)
- SPI.lastError(), SPI.isInitialized() (preview diagnostic functions)

Use **software CS**: digitalWrite(PD(0), LOW) before transferring a
complete command and data, then digitalWrite(PD(0), HIGH) afterward.
SPI.beginTransaction() configures clock and mode but does not assert CS,
matching the Arduino SPI transaction pattern. SPI uses 8-bit polling,
without interrupts, DMA, FIFO, or hardware SS.

## Flash JEDEC ID (read-only)

    #include <SPI.h>
    void setup() {
      Serial.begin(115200);
      SPI.begin();
    }
    void loop() {
      SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
      digitalWrite(PD(0), LOW);
      SPI.transfer(0x9F);
      uint8_t manufacturer = SPI.transfer(0);
      uint8_t type = SPI.transfer(0);
      uint8_t capacity = SPI.transfer(0);
      digitalWrite(PD(0), HIGH);
      SPI.endTransaction();
      Serial.print("JEDEC: ");
      Serial.print(manufacturer, HEX);
      Serial.print(" ");
      Serial.print(type, HEX);
      Serial.print(" ");
      Serial.println(capacity, HEX);
      delay(2000);
    }

For the full example with status/error checks, use File > Examples > SPI >
SPIFlash_JEDEC_ID. Status-register read-only example is also included.

## Installation (installed Boards Manager core v0.1.2)

Unzip the separate Windows installer archive; close Arduino IDE;
double-click INSTALL-SPI.cmd. It copies the **SPI** library into:
\`%LOCALAPPDATA%\Arduino15\packages\nuc140\hardware\nuc140\0.1.2\libraries\SPI\`

No modifications to Wire.h, HardwareSerial, analogRead, Nu-Link uploader
or platform.txt. Board Manager updates may overwrite manually installed
libraries until the core is released with the new library.

## Constraints

- SPI2 only, 8-bit transfers, master mode.
- Assumes the experimental core's HCLK of 22.1184 MHz.
- Default clock 1MHz; unverified higher speeds.
- \`lastError()\` 0=OK, 1=protected-register unlock failed,
  2/3=SPI busy timeout; these are NUC140 preview extensions.
- Does not erase or program Flash in example sketches.
- SPI library not yet hardware-verified; JEDEC identity must be checked
  against device markings.

References:
- https://github.com/OpenNuvoton/NUC100BSP
- https://www.nuvoton.com/resource-files/UM_Nu-LB-NUC140_EN_Rev2.0.pdf
