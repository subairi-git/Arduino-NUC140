#pragma once
#include <stdint.h>
#include <stddef.h>

// SPI2 on Nu-LB-NUC140 V2.0: PD0 manual CS, PD1 SCK, PD2 MISO0, PD3 MOSI0.
// Master mode with polling; this experimental core currently offers only SPI2.
#ifndef SPI_HAS_TRANSACTION
#define SPI_HAS_TRANSACTION 1
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif
#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef SPI_MODE0
#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C
#endif

class SPISettings {
public:
  SPISettings(uint32_t clock = 1000000UL, uint8_t bitOrder = MSBFIRST,
              uint8_t dataMode = SPI_MODE0)
    : clock_(clock), bitOrder_(bitOrder), dataMode_(dataMode) {}
  uint32_t clock() const { return clock_; }
  uint8_t bitOrder() const { return bitOrder_; }
  uint8_t dataMode() const { return dataMode_; }
private:
  uint32_t clock_;
  uint8_t bitOrder_;
  uint8_t dataMode_;
};

class SPIClass {
public:
  SPIClass();
  void begin();
  void end();
  void beginTransaction(const SPISettings& settings);
  void endTransaction();
  uint8_t transfer(uint8_t data);
  uint16_t transfer16(uint16_t data);
  void transfer(void* buffer, size_t count);

  // NUC140-specific status: 0 OK; 1 protected registers; 2/3 timeout.
  uint8_t lastError() const { return error_; }
  bool isInitialized() const { return initialized_; }
private:
  void configure(const SPISettings& settings);
  bool initialized_;
  uint8_t error_;
  SPISettings current_;
};

extern SPIClass SPI;
