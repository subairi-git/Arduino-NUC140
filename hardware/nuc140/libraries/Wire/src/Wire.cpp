#include "Wire.h"

/*
 * Register addresses and state codes from OpenNuvoton/NUC100BSP:
 * Library/Device/Nuvoton/NUC100Series/Include/NUC100Series.h
 * Library/StdDriver/inc/i2c.h, src/i2c.c
 * SampleCode/StdDriver/I2C_Master/main.c
 *
 * This lightweight Wire implementation uses the known-working I2C1
 * configuration on Nu-LB-NUC140 V2.0 (PA10/PA11, EEPROM 24LC64).
 * It does not change UART0, ADC7, HCLK or the Nu-Link uploader.
 */
#define WIRE_REG32(address) (*reinterpret_cast<volatile uint32_t *>(address))

static const uint32_t REG_SYS_PROTECT = 0x50000100UL;
static const uint32_t REG_GPA_MFP     = 0x50000030UL;
static const uint32_t REG_ALT_MFP    = 0x50000050UL;
static const uint32_t REG_CLK_APBCLK = 0x50000208UL;

static const uint32_t I2C1_BASE_ADDR = 0x40120000UL;
static const uint32_t REG_I2C1_CTL   = I2C1_BASE_ADDR + 0x00UL;
static const uint32_t REG_I2C1_DAT   = I2C1_BASE_ADDR + 0x08UL;
static const uint32_t REG_I2C1_STATUS= I2C1_BASE_ADDR + 0x0CUL;
static const uint32_t REG_I2C1_CLK   = I2C1_BASE_ADDR + 0x10UL;

/* Follows the clock assumption in the experimental NUC140 core (HCLK/PCLK). */
static const uint32_t PCLK_HZ = 22118400UL;
static const uint32_t WAIT_LIMIT = 1000000UL;

TwoWire Wire;

TwoWire::TwoWire()
  : initialized_(false), bus_held_(false), tx_overflow_(false),
    tx_address_(0), tx_count_(0), rx_count_(0), rx_index_(0),
    last_status_(0xF8), bus_clock_hz_(100000UL) {}

void TwoWire::begin() {
  /* Protect the already-working bus from accidental reinitialization. */
  if (initialized_) return;
  WIRE_REG32(REG_SYS_PROTECT) = 0x59UL;
  WIRE_REG32(REG_SYS_PROTECT) = 0x16UL;
  WIRE_REG32(REG_SYS_PROTECT) = 0x88UL;
  if ((WIRE_REG32(REG_SYS_PROTECT) & 1UL) == 0UL) {
    last_status_ = 0xFF;
    return;
  }

  WIRE_REG32(REG_CLK_APBCLK) |= (1UL << 9); // I2C1 APB clock
  WIRE_REG32(REG_ALT_MFP) &= ~(1UL << 11); // Disable conflicting EBI function
  WIRE_REG32(REG_GPA_MFP) |= (1UL << 10) | (1UL << 11);
  WIRE_REG32(REG_SYS_PROTECT) = 0UL;

  /* I2C1 controls its pin signals when ENS1 is set. */
  WIRE_REG32(REG_I2C1_CTL) = 0UL;
  initialized_ = true;
  bus_held_ = false;
  last_status_ = 0xF8;
  setClock(bus_clock_hz_);
  WIRE_REG32(REG_I2C1_CTL) = (1UL << 6); // ENS1, interrupts disabled
}

void TwoWire::end() {
  if (!initialized_) return;
  if (bus_held_) stop();
  WIRE_REG32(REG_I2C1_CTL) = 0UL;
  bus_held_ = false;
  initialized_ = false;
}

void TwoWire::setClock(uint32_t clock_hz) {
  if (clock_hz < 10000UL) clock_hz = 10000UL;
  if (clock_hz > 400000UL) clock_hz = 400000UL;
  bus_clock_hz_ = clock_hz;
  if (!initialized_) return;
  // SCL = PCLK / [4 * (I2CLK + 1)], minimum I2CLK is 4.
  uint32_t divisor = (PCLK_HZ + 2UL * clock_hz) / (4UL * clock_hz);
  if (divisor < 5UL) divisor = 5UL;
  if (divisor > 256UL) divisor = 256UL;
  WIRE_REG32(REG_I2C1_CLK) = divisor - 1UL;
}

void TwoWire::control(uint32_t command) {
  WIRE_REG32(REG_I2C1_CTL) =
    (WIRE_REG32(REG_I2C1_CTL) & ~0x3CUL) | command;
}

bool TwoWire::awaitStatus(uint8_t expected) {
  uint32_t n = WAIT_LIMIT;
  while ((WIRE_REG32(REG_I2C1_CTL) & 0x08UL) == 0UL && n) --n;
  if (n == 0UL) {
    last_status_ = 0xFF; // bounded timeout
    return false;
  }
  last_status_ = static_cast<uint8_t>(WIRE_REG32(REG_I2C1_STATUS));
  return last_status_ == expected;
}

bool TwoWire::start() {
  /* Initial START -> 0x08; repeated START -> 0x10. */
  const bool repeated = bus_held_;
  control(repeated ? 0x28UL : 0x20UL);
  return awaitStatus(repeated ? 0x10U : 0x08U);
}

bool TwoWire::sendByte(uint8_t data, uint8_t expected) {
  WIRE_REG32(REG_I2C1_DAT) = data;
  control(0x08UL); // Clear SI, continue transaction
  return awaitStatus(expected);
}

void TwoWire::stop() {
  if (!initialized_) return;
  control(0x18UL); // STOP + SI, controller releases bus
  uint32_t n = WAIT_LIMIT;
  while ((WIRE_REG32(REG_I2C1_CTL) & 0x10UL) && n) --n;
  if (n == 0UL) last_status_ = 0xFF;
  bus_held_ = false;
}

void TwoWire::beginTransmission(uint8_t address) {
  if (!initialized_) begin();
  tx_address_ = address;
  tx_count_ = 0;
  tx_overflow_ = false;
}

size_t TwoWire::write(uint8_t data) {
  if (tx_count_ >= kBufferSize) {
    tx_overflow_ = true;
    return 0;
  }
  tx_buffer_[tx_count_++] = data;
  return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity) {
  if (!data) return 0;
  size_t written = 0;
  for (; written < quantity; ++written) {
    if (!write(data[written])) break;
  }
  return written;
}

size_t TwoWire::write(const char *str) {
  if (!str) return 0;
  size_t n = 0;
  while (*str) {
    if (!write(static_cast<uint8_t>(*str++))) break;
    ++n;
  }
  return n;
}

uint8_t TwoWire::endTransmission(uint8_t sendStop) {
  if (!initialized_) return 4;
  if (tx_overflow_) {
    if (bus_held_) stop();
    tx_count_ = 0;
    return 1; // Arduino Wire: data too long
  }
  if (!start()) {
    stop();
    tx_count_ = 0;
    return 4; // other error / timeout
  }
  if (!sendByte(static_cast<uint8_t>(tx_address_ << 1), 0x18U)) {
    const uint8_t result = (last_status_ == 0x20U) ? 2U : 4U;
    stop();
    tx_count_ = 0;
    return result; // address NACK / other
  }
  for (uint8_t i = 0; i < tx_count_; ++i) {
    if (!sendByte(tx_buffer_[i], 0x28U)) {
      const uint8_t result = (last_status_ == 0x30U) ? 3U : 4U;
      stop();
      tx_count_ = 0;
      return result; // data NACK / other
    }
  }
  tx_count_ = 0;
  if (sendStop) stop();
  else bus_held_ = true; // keep bus for EEPROM repeated START
  return 0;
}

size_t TwoWire::requestFrom(uint8_t address, size_t quantity, bool sendStop) {
  rx_count_ = 0;
  rx_index_ = 0;
  if (!initialized_) begin();
  if (!initialized_ || quantity == 0) return 0;
  if (quantity > kBufferSize) quantity = kBufferSize;

  if (!start()) {
    stop();
    return 0;
  }
  if (!sendByte(static_cast<uint8_t>((address << 1) | 1U), 0x40U)) {
    stop();
    return 0;
  }
  for (size_t i = 0; i < quantity; ++i) {
    const bool ack = (i + 1U < quantity);
    control(ack ? 0x0CUL : 0x08UL); // ACK except final byte (NACK)
    if (!awaitStatus(ack ? 0x50U : 0x58U)) {
      stop();
      rx_count_ = 0;
      return 0;
    }
    rx_buffer_[rx_count_++] =
      static_cast<uint8_t>(WIRE_REG32(REG_I2C1_DAT) & 0xFFUL);
  }
  if (sendStop) stop();
  else bus_held_ = true;
  return rx_count_;
}

int TwoWire::available() {
  return static_cast<int>(rx_count_ - rx_index_);
}

int TwoWire::peek() {
  if (rx_index_ >= rx_count_) return -1;
  return rx_buffer_[rx_index_];
}

int TwoWire::read() {
  if (rx_index_ >= rx_count_) return -1;
  return rx_buffer_[rx_index_++];
}

void TwoWire::flush() {
  /* Synchronous polling: all TX bytes are sent by endTransmission(). */
}
