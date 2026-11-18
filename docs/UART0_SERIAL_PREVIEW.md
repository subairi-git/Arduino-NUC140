# NUC140 UART0 Serial preview

Fix for `error: 'Serial' was not declared in this scope`.

This experimental adapter provides `Serial.begin(baud)`, `Serial.print()`,
`Serial.println()`, `available()`, `peek()`, `read()`, `write()`,
`flush()`, `end()`, and a subset of Arduino `Print` number formatting.

**Hardware is NOT tested yet.** A successful compile does not prove receive,
transmit timing, framing, or electrical compatibility.

## NUC140VE3CN mapping

| Signal | Pin | Arduino pin |
|---|---|---|
| UART0 RXD | PB0 | 16 |
| UART0 TXD | PB1 | 17 |

Clock: HIRC 22.1184 MHz, UART divider 1. Default UART format: 8N1.
First implementation uses polling, not interrupts or buffered RX.
Write loops use bounded waits to avoid indefinite blocking if the
peripheral is not responding.

## Smoke test

1. Arduino IDE `File > Examples > Nuc140Examples > SerialHello`.
2. Compile and check that `Serial` is now declared.
3. Verify the target is **NUC140VE3CN** before Upload.
4. Only connect suitable logic-level serial hardware and a common GND;
   do not connect a +/-12V RS-232 port to MCU pins.
5. Open the host serial monitor for the USB-to-UART adapter at 115200 8N1.

Nu-Link SWD programming is separate from UART communications.
The programming cable is **not automatically a serial COM port**.

## Known limitations

This is not a full Arduino `Stream`/`Print` implementation.
`Serial.available()` reports 0 or 1, not exact FIFO depth.
`Serial.print(String)` and other Arduino advanced API overloads
are not implemented yet. Multiple UARTs and DMA/IRQ reception will follow
after the basic UART0 hardware test.

Source basis: official Nuvoton NUC100Series.h, UART driver and sample,
https://github.com/OpenNuvoton/NUC100BSP (see upstream license).
