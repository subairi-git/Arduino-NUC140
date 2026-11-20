# Wire.h I2C1 experimental preview — NUC140VE3CN

This is an experimental Arduino-style Wire master library for the
Nu-LB-NUC140 V2.0 board with NUC140VE3CN. The hardware was previously
verified using a **register-level I2C1 sketch**: EEPROM 24LC64 at 0x50
ACKed and returned the 16 bytes 05 06 07 08 09 0A 0B 0C 0D 0E 0F
10 11 12 13 14. The **Wire library itself is not yet hardware-tested**.

The newly added library lives in:
hardware/nuc140/libraries/Wire

## API

- Wire.begin() / Wire.end()
- Wire.setClock(100000)
- Wire.beginTransmission(address)
- Wire.write(byte), Wire.write(buffer, size)
- Wire.endTransmission(stop = true)
- Wire.requestFrom(address, quantity, stop = true)
- Wire.available(), Wire.read(), Wire.peek()
- Wire.lastStatus() — NUC140-specific diagnostic

Default Wire controls **I2C1** on PA10 SDA / PA11 SCL.
EEPROM 24LC64 responds to 7-bit address **0x50**.
One 32-byte transmit buffer and one 32-byte receive buffer; requestFrom
silently clamps requests larger than 32 bytes. Polling only, master only;
no I2C slave callbacks/interrupts, no Wire1 / I2C0, no DMA.

The library uses HCLK/PCLK **assumed** to be 22.1184 MHz, inherited from
the current experimental core. It does not reconfigure the UART/ADC
clocks. Fast-mode (400kHz) has not been tested on hardware. START,
repeated START, ACK/NACK, STOP, and bounded waits mirror the NUC100 BSP.

## Quick sketch

    #include <Wire.h>
    void setup() {
      Serial.begin(115200);
      Wire.begin();
      Wire.setClock(100000);
    }
    void loop() {
      Wire.beginTransmission(0x50);
      if (Wire.endTransmission() == 0) {
        Serial.println("EEPROM FOUND");
      }
      delay(1000);
    }

For a complete **read-only** EEPROM test, use
Wire/examples/EEPROM24LC64_ReadOnly/EEPROM24LC64_ReadOnly.ino
or Wire/examples/I2C_Scanner/I2C_Scanner.ino.

## Windows installer (Board Manager 0.1.2)

A standalone ZIP installer is provided separately. It copies only
the Wire folder to:

    %LOCALAPPDATA%\Arduino15\packages\nuc140\hardware\nuc140\0.1.2\libraries\Wire

Stop Arduino IDE, copy the folder, reopen Arduino IDE, select NUC140 V2.0,
upload the example and open the UART0 Serial Monitor at 115200 baud.
The installer does NOT overwrite the working Serial, analogRead,
Nu-Link uploader or platform.txt. Boards Manager updates could overwrite
manually installed files until a new tagged package is published.

## Current limitations

- Not yet verified on hardware as a library.
- Default I2C1 only; I2C0 and additional devices not supported yet.
- Error codes match Arduino Wire conventional success 0, overflow 1,
  address NACK 2, data NACK 3, timeout/other 4.
- Master mode only; small 32-byte buffers.
- No synchronization across concurrent users; not interrupt-safe.
- The EEPROM example only **reads**, it does not change contents.

Register source: https://github.com/OpenNuvoton/NUC100BSP
