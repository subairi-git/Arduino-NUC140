#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * Experimental TwoWire master adapter for Nu-LB-NUC140 V2.0.
 * Wire maps to I2C1 on PA10 (SDA), PA11 (SCL).
 * Master mode only; polling and bounded waits, 32-byte buffers.
 */
class TwoWire {
public:
  TwoWire();

  void begin();
  void end();
  void setClock(uint32_t clock_hz);

  void beginTransmission(uint8_t address);
  size_t write(uint8_t data);
  size_t write(const uint8_t *data, size_t quantity);
  size_t write(const char *str);
  uint8_t endTransmission(uint8_t sendStop = 1);

  size_t requestFrom(uint8_t address, size_t quantity, bool sendStop = true);
  int available();
  int read();
  int peek();
  void flush();

  /* Nuvoton-specific diagnostics; 0xFF means bus-step timeout. */
  uint8_t lastStatus() const { return last_status_; }
  bool isInitialized() const { return initialized_; }

private:
  enum { kBufferSize = 32 };
  bool initialized_;
  bool bus_held_;
  bool tx_overflow_;
  uint8_t tx_address_;
  uint8_t tx_buffer_[kBufferSize];
  uint8_t tx_count_;
  uint8_t rx_buffer_[kBufferSize];
  uint8_t rx_count_;
  uint8_t rx_index_;
  uint8_t last_status_;
  uint32_t bus_clock_hz_;

  void control(uint32_t command);
  bool awaitStatus(uint8_t expected);
  bool start();
  bool sendByte(uint8_t data, uint8_t expected);
  void stop();
};

extern TwoWire Wire;
