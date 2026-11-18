#pragma once
/*
 * NUC140VE3CN peripheral alternate-function PIN CANDIDATES.
 *
 * Source: OpenNuvoton/NUC100BSP StdDriver examples, Apache-2.0.
 * Values follow the local Arduino core numbering convention:
 * PA(n)=n, PB(n)=16+n, PC(n)=32+n.
 *
 * These definitions do NOT set multi-function pin mux registers.
 * Physical access and conflict-free routing on Nu-LB-NUC140 V2.0
 * must be checked against the board schematic.
 */
#define PIN_SERIAL_RX 16  // PB0 UART0_RXD
#define PIN_SERIAL_TX 17  // PB1 UART0_TXD
#define PIN_WIRE_SDA 8    // PA8 I2C0_SDA
#define PIN_WIRE_SCL 9    // PA9 I2C0_SCL
#define PIN_SPI_SS 40    // PC8 SPI1_SS0
#define PIN_SPI_SCK 41   // PC9 SPI1_CLK
#define PIN_SPI_MISO 42  // PC10 SPI1_MISO0
#define PIN_SPI_MOSI 43  // PC11 SPI1_MOSI0
#define PIN_ADC0 0       // PA0 ADC0
#define PIN_ADC1 1       // PA1 ADC1
#define PIN_ADC2 2       // PA2 ADC2
#define PIN_ADC3 3       // PA3 ADC3
