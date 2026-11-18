# NuMaker UNO → Nu-LB-NUC140 V2.0 porting study

Status: **experimental research branch; peripheral APIs NOT implemented yet**. Keep the proven Blink/Nu-Link release on `main` unchanged until target-specific APIs are reviewed and tested.

## Official upstreams

1. [OpenNuvoton/NuMaker_UNO](https://github.com/OpenNuvoton/NuMaker_UNO), official Arduino platform index (`NuMaker UNO` version 3.1.8). The repository tracks a **46,182,122-byte tar.bz2** rather than unpacked source files. Listed targets are NUC131, M032, M252 and M487, **not NUC140VE3CN**.
2. [OpenNuvoton/NUC100BSP](https://github.com/OpenNuvoton/NUC100BSP): NUC100 CMSIS/StdDriver, examples and startup reference. This is the appropriate device-family layer for NUC140. BSP README indicates **Apache-2.0**; retain license headers and inspect third-party licenses before redistribution.
3. [Current Arduino NUC140 core](https://github.com/subairi-git/Arduino-NUC140): working proof of concept for pin PC12 Blink and Windows Nu-Link APROM upload.

**Do not extract the official NuMaker binary into the shipping Arduino package without inspecting its contents, component licenses and MCU compatibility.** The source-audit workflow only lists the upstream archive's member paths after checking its SHA-256; it never executes code from that archive.

## Hardware mapping candidates (NUC100 BSP sample code)

These are the **chip alternate-function candidates**, not a confirmed external connector diagram for Nu-LB-NUC140 V2.0. Check board schematic and multiplexed on-board devices before applying these pins.

| Arduino API (planned) | Target module | Peripheral pins | Local Arduino pin values | Upstream BSP sample |
| --- | --- | --- | --- | --- |
| `Serial` | UART0 | RXD PB0, TXD PB1 | 16, 17 | `UART_TxRx_Function/main.c` |
| `Wire` | I2C0 | SDA PA8, SCL PA9 | 8, 9 | `I2C_Master/main.c` |
| `SPI` | SPI1 | SS PC8, SCK PC9, MISO PC10, MOSI PC11 | 40, 41, 42, 43 | `SPI_Loopback/main.c` |
| `analogRead` (first 4 channels) | ADC | ADC0–3 PA0–PA3 | 0–3 | `ADC_SingleMode/main.c` |
| `LED_BUILTIN` | GPIO | PC12 | 44 | already tested on the user's board |

Source sample paths are inside `OpenNuvoton/NUC100BSP/SampleCode/StdDriver/`.

The file `hardware/nuc140/variants/nuc140v2/peripheral_pins.h` records these numeric constants but does **not** turn on UART/I2C/SPI/ADC hardware.

## Work needed before Arduino peripherals actually work

- **Clock:** configure HIRC, clocks for each peripheral, and confirm HCLK and baud/ADC clock dividers. Current core assumes 22.1184 MHz from reset.
- **Multi-function I/O:** switch each pin's mux to peripheral function, preserve unrelated mux bits and don't break onboard LCD/keypad/LED usage.
- **Startup and IRQ:** replace current all-default external IRQ slots with the verified NUC100/NUC140 vector layout. The current 32-slot default table is not a usable interrupt controller for full peripheral drivers.
- **CMSIS:** include device headers, NVIC and register types from the NUC100 BSP, with correct toolchain include paths and module dependencies.
- **Arduino adaptation:** implement `HardwareSerial`, `TwoWire`, `SPIClass`, `analogRead()`, `analogWrite()`, interrupts and drivers using NUC100 StdDriver functions and the board pin map; keep symbols in sync with Arduino headers and linker.
- **Release:** keep Nu-Link uploader and packaging until a real board verifies the replacements; don't publish a Boards Manager version simply because code compiles.
- **Tests:** Blink regression, UART0 TX/RX loopback, I2C master with a known responding device, SPI1 loopback, ADC ground/reference test, PWM oscilloscope/LED observation, interrupts/timer and sleep/reset.

## Test status

| Capability | NUC140 board test |
| --- | --- |
| Blink PC12 using existing core | **PASS**, reported by hardware owner |
| Nu-Link firmware write/verify | **PASS**, reported by hardware owner |
| UART0 `Serial` | **NOT IMPLEMENTED / NOT TESTED** |
| I2C0 `Wire` | **NOT IMPLEMENTED / NOT TESTED** |
| SPI1 `SPI` | **NOT IMPLEMENTED / NOT TESTED** |
| ADC/PWM/Timer/RTC/USB | **NOT IMPLEMENTED / NOT TESTED** |

### Sources

- https://github.com/OpenNuvoton/NuMaker_UNO
- https://github.com/OpenNuvoton/NUC100BSP/tree/master/Library/StdDriver
- https://github.com/OpenNuvoton/NUC100BSP/tree/master/SampleCode/StdDriver
