#include "Arduino.h"
#include "SPI.h"

/*
 * NUC140 SPI2 master for the lower onboard W25X16 SPI Flash (U3):
 * PD0 = software CS (GPIO), PD1 = SPI2_CLK, PD2 = SPI2_MISO0,
 * PD3 = SPI2_MOSI0. All other PD pins untouched.
 *
 * Nuvoton NUC100BSP / NUC100Series.h, spi.c, sys.h, clk.h.
 * No UART, ADC, I2C or system core clock changes.
 */
#define SPI_R32(addr) (*reinterpret_cast<volatile uint32_t*>(addr))
static const uint32_t REG_PROTECT   = 0x50000100UL;
static const uint32_t REG_GPD_MFP   = 0x5000003CUL;
static const uint32_t REG_APBCLK    = 0x50000208UL;
static const uint32_t REG_CLKSEL1   = 0x50000214UL;

static const uint32_t REG_SPI2_CTL  = 0x40130000UL;
static const uint32_t REG_SPI2_DIV  = 0x40130004UL;
static const uint32_t REG_SPI2_SS   = 0x40130008UL;
static const uint32_t REG_SPI2_RX0  = 0x40130010UL;
static const uint32_t REG_SPI2_TX0  = 0x40130020UL;
static const uint32_t REG_SPI2_CTL2 = 0x4013003CUL;

static const uint32_t SPI_CLOCK_HZ = 22118400UL; // experimental core HCLK
static const uint32_t WAIT_LIMIT   = 1000000UL;
static const uint32_t SPI_GO_BUSY  = 1UL;

SPIClass SPI;
SPIClass::SPIClass() : initialized_(false), error_(0), current_() {}

void SPIClass::begin() {
  if (initialized_) return;
  error_ = 0;

  // Clock routing + GPD multi-function are protected registers.
  SPI_R32(REG_PROTECT) = 0x59UL;
  SPI_R32(REG_PROTECT) = 0x16UL;
  SPI_R32(REG_PROTECT) = 0x88UL;
  if ((SPI_R32(REG_PROTECT) & 1UL) == 0UL) {
    error_ = 1;
    return;
  }
  SPI_R32(REG_APBCLK) |= (1UL << 14); // SPI2 clock enable
  SPI_R32(REG_CLKSEL1) |= (1UL << 6); // SPI2 clock source HCLK

  // PD0 is ordinary GPIO CS; PD1/PD2/PD3 are SPI2 peripheral signals.
  SPI_R32(REG_GPD_MFP) =
    (SPI_R32(REG_GPD_MFP) & ~(0xFUL)) | (0xEUL);
  SPI_R32(REG_PROTECT) = 0UL;

  pinMode(PD(0), OUTPUT);
  digitalWrite(PD(0), HIGH);     // deassert CS before starting SPI2

  SPI_R32(REG_SPI2_SS) = 0UL;  // Hardware chip-select disabled
  SPI_R32(REG_SPI2_CTL2) = (1UL << 31); // BCn=1, SPI_CLK/(DIV+1)
  SPI_R32(REG_SPI2_CTL) = 0UL;  // No FIFO / IRQ / dual-bit / slave mode
  initialized_ = true;
  configure(current_);
}

void SPIClass::end() {
  if (!initialized_) return;
  uint32_t count = WAIT_LIMIT;
  while ((SPI_R32(REG_SPI2_CTL) & SPI_GO_BUSY) && count) --count;
  if (!count) error_ = 2;
  digitalWrite(PD(0), HIGH);
  SPI_R32(REG_SPI2_CTL) = 0UL;
  initialized_ = false;
}

void SPIClass::configure(const SPISettings& settings) {
  current_ = settings;
  if (!initialized_) return;
  uint32_t hz = settings.clock();
  if (hz == 0UL) hz = 1000000UL;
  // Use ceil division to avoid exceeding the requested clock.
  uint32_t div = (SPI_CLOCK_HZ + hz - 1UL) / hz;
  if (div < 1UL) div = 1UL;
  uint32_t clock_mode = (1UL << 31); // BCn=1, /1..256
  if (div > 256UL) {
    clock_mode = 0UL;              // BCn=0, /2..512
    const uint32_t doubled = hz * 2UL;
    div = (SPI_CLOCK_HZ + doubled - 1UL) / doubled;
  }
  if (div > 256UL) div = 256UL;
  SPI_R32(REG_SPI2_CTL2) = clock_mode;
  SPI_R32(REG_SPI2_DIV) = (div - 1UL) & 0xFFUL;

  uint32_t ctl = 8UL << 3;        // 8 data bits per SPI word
  switch (settings.dataMode() & 0x0CUL) {
    case SPI_MODE1: ctl |= (1UL << 1); break;           // RX_NEG
    case SPI_MODE2: ctl |= (1UL << 11) | (1UL << 1); break;
    case SPI_MODE3: ctl |= (1UL << 11) | (1UL << 2); break;
    default:        ctl |= (1UL << 2); break;           // Mode 0 TX_NEG
  }
  if (settings.bitOrder() == LSBFIRST) ctl |= (1UL << 10);
  SPI_R32(REG_SPI2_CTL) = ctl;
}

void SPIClass::beginTransaction(const SPISettings& settings) {
  if (!initialized_) begin();
  if (!initialized_) return;
  error_ = 0;
  uint32_t n = WAIT_LIMIT;
  while ((SPI_R32(REG_SPI2_CTL) & SPI_GO_BUSY) && n) --n;
  if (!n) { error_ = 2; return; }
  configure(settings);
}

void SPIClass::endTransaction() {
  // Standard Arduino: caller controls CS using digitalWrite(PD(0), LOW/HIGH).
}

uint8_t SPIClass::transfer(uint8_t data) {
  if (!initialized_) begin();
  if (!initialized_ || error_ != 0) return 0xFFU;
  uint32_t n = WAIT_LIMIT;
  while ((SPI_R32(REG_SPI2_CTL) & SPI_GO_BUSY) && n) --n;
  if (!n) { error_ = 2; return 0xFFU; }
  SPI_R32(REG_SPI2_TX0) = data;
  SPI_R32(REG_SPI2_CTL) |= SPI_GO_BUSY;
  n = WAIT_LIMIT;
  while ((SPI_R32(REG_SPI2_CTL) & SPI_GO_BUSY) && n) --n;
  if (!n) { error_ = 3; return 0xFFU; }
  return static_cast<uint8_t>(SPI_R32(REG_SPI2_RX0) & 0xFFUL);
}

uint16_t SPIClass::transfer16(uint16_t data) {
  if (current_.bitOrder() == LSBFIRST) {
    uint8_t low = transfer(static_cast<uint8_t>(data));
    uint8_t high = transfer(static_cast<uint8_t>(data >> 8));
    return static_cast<uint16_t>((static_cast<uint16_t>(high) << 8) | low);
  }
  uint8_t high = transfer(static_cast<uint8_t>(data >> 8));
  uint8_t low = transfer(static_cast<uint8_t>(data));
  return static_cast<uint16_t>((static_cast<uint16_t>(high) << 8) | low);
}

void SPIClass::transfer(void* buffer, size_t count) {
  if (!buffer) return;
  uint8_t *bytes = static_cast<uint8_t*>(buffer);
  for (size_t i = 0; i < count; ++i) bytes[i] = transfer(bytes[i]);
}
