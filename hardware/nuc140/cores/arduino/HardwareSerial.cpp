#include "Arduino.h"
#include "HardwareSerial.h"

// Registers and bit positions checked against OpenNuvoton/NUC100BSP
// NUC100Series.h, uart.c and UART_TxRx_Function/main.c (Apache-2.0).
// This standalone polling adapter is authored for the Arduino NUC140 core.
#define MMIO32(address) (*reinterpret_cast<volatile uint32_t *>(address))
static const uint32_t REG_GPB_MFP     = 0x50000034UL;
static const uint32_t REG_PROTECT     = 0x50000100UL;
static const uint32_t REG_PWRCON      = 0x50000200UL;
static const uint32_t REG_APBCLK      = 0x50000208UL;
static const uint32_t REG_CLKSTATUS   = 0x5000020CUL;
static const uint32_t REG_CLKSEL1     = 0x50000214UL;
static const uint32_t REG_CLKDIV      = 0x50000218UL;
static const uint32_t UART0_BASE_ADDR = 0x40050000UL;
static const uint32_t REG_UART_DATA   = UART0_BASE_ADDR + 0x00UL;
static const uint32_t REG_UART_IER    = UART0_BASE_ADDR + 0x04UL;
static const uint32_t REG_UART_FCR    = UART0_BASE_ADDR + 0x08UL;
static const uint32_t REG_UART_LCR    = UART0_BASE_ADDR + 0x0CUL;
static const uint32_t REG_UART_FSR    = UART0_BASE_ADDR + 0x18UL;
static const uint32_t REG_UART_BAUD   = UART0_BASE_ADDR + 0x24UL;
static const uint32_t REG_UART_FUNC   = UART0_BASE_ADDR + 0x30UL;
static const uint32_t UART0_CLOCK_HZ  = 22118400UL;
static const uint32_t POLL_LIMIT      = 2000000UL;
static const uint32_t UART_TX_FULL    = 1UL << 23;
static const uint32_t UART_RX_EMPTY   = 1UL << 14;
static const uint32_t UART_TX_IDLE    = 1UL << 28;

HardwareSerial Serial;
HardwareSerial::HardwareSerial() : started_(false), peeked_(-1) {}

void HardwareSerial::begin(uint32_t baud) {
  started_ = false;
  peeked_ = -1;
  if (baud == 0UL) return;

  // UART_BAUD_MODE2_DIVIDER from official BSP: UART_CLK / baud - 2.
  uint32_t divider = (UART0_CLOCK_HZ + (baud / 2UL)) / baud;
  uint32_t baudReg;
  if (divider >= 2UL && divider <= 65537UL) {
    baudReg = (1UL << 29) | (1UL << 28) | (divider - 2UL);
  } else if (divider > 65537UL) {
    uint32_t slow = ((UART0_CLOCK_HZ / 16UL) + (baud / 2UL)) / baud;
    if (slow < 2UL || slow > 65537UL) return;
    baudReg = slow - 2UL;
  } else {
    return;
  }

  // Unlock protected clock configuration registers (Nuvoton SYS_UnlockReg).
  MMIO32(REG_PROTECT) = 0x59UL;
  MMIO32(REG_PROTECT) = 0x16UL;
  MMIO32(REG_PROTECT) = 0x88UL;
  if ((MMIO32(REG_PROTECT) & 1UL) == 0UL) return;

  // Select HIRC 22.1184 MHz as the UART clock; leave HCLK untouched.
  MMIO32(REG_PWRCON) |= (1UL << 2);
  uint32_t wait = POLL_LIMIT;
  while ((MMIO32(REG_CLKSTATUS) & (1UL << 4)) == 0UL && wait) --wait;
  if (!wait) {
    MMIO32(REG_PROTECT) = 0UL;
    return;
  }
  MMIO32(REG_APBCLK) |= (1UL << 16); // UART0 clock
  MMIO32(REG_CLKSEL1) = (MMIO32(REG_CLKSEL1) & ~(3UL << 24)) | (3UL << 24);
  MMIO32(REG_CLKDIV) &= ~(15UL << 8); // UART clock divider = 1
  MMIO32(REG_GPB_MFP) |= 3UL;        // PB0 RXD / PB1 TXD
  MMIO32(REG_PROTECT) = 0UL;

  MMIO32(REG_UART_IER) = 0UL;   // Polling, no external IRQ dependency
  MMIO32(REG_UART_FUNC) = 0UL;  // UART (not IrDA or RS485)
  MMIO32(REG_UART_LCR) = 3UL;   // 8N1
  MMIO32(REG_UART_FCR) = (1UL << 1) | (1UL << 2); // Reset FIFOs
  MMIO32(REG_UART_BAUD) = baudReg;
  started_ = true;
}

void HardwareSerial::end() {
  flush();
  if (started_) MMIO32(REG_UART_IER) = 0UL;
  // Keep clock/pin settings unchanged in case other modules use them.
  started_ = false;
  peeked_ = -1;
}
int HardwareSerial::available() {
  if (!started_) return 0;
  // Conservative 0/1 result; RX FIFO count not exposed yet.
  return (peeked_ >= 0 || (MMIO32(REG_UART_FSR) & UART_RX_EMPTY) == 0UL) ? 1 : 0;
}
int HardwareSerial::availableForWrite() {
  if (!started_) return 0;
  return (MMIO32(REG_UART_FSR) & UART_TX_FULL) ? 0 : 1;
}
int HardwareSerial::read() {
  if (!started_) return -1;
  if (peeked_ >= 0) {
    const int value = peeked_;
    peeked_ = -1;
    return value;
  }
  if (MMIO32(REG_UART_FSR) & UART_RX_EMPTY) return -1;
  return static_cast<int>(MMIO32(REG_UART_DATA) & 0xFFUL);
}
int HardwareSerial::peek() {
  if (!started_) return -1;
  if (peeked_ < 0) peeked_ = read();
  return peeked_;
}
void HardwareSerial::flush() {
  if (!started_) return;
  uint32_t wait = POLL_LIMIT;
  while ((MMIO32(REG_UART_FSR) & UART_TX_IDLE) == 0UL && wait) --wait;
}
size_t HardwareSerial::write(uint8_t byte) {
  if (!started_) return 0;
  uint32_t wait = POLL_LIMIT;
  while ((MMIO32(REG_UART_FSR) & UART_TX_FULL) && wait) --wait;
  if (!wait) return 0;
  MMIO32(REG_UART_DATA) = byte;
  return 1;
}
size_t HardwareSerial::write(const uint8_t *buffer, size_t length) {
  if (!buffer) return 0;
  size_t sent = 0;
  while (sent < length && write(buffer[sent])) ++sent;
  return sent;
}
size_t HardwareSerial::write(const char *str) {
  if (!str) return 0;
  size_t result = 0;
  while (*str) {
    if (!write(static_cast<uint8_t>(*str++))) break;
    ++result;
  }
  return result;
}
size_t HardwareSerial::print(const char *str) { return write(str); }
size_t HardwareSerial::print(char value) { return write(static_cast<uint8_t>(value)); }
size_t HardwareSerial::printUnsigned(unsigned long value, int base) {
  if (base == 0) return write(static_cast<uint8_t>(value));
  if (base < 2 || base > 36) base = DEC;
  char buf[8 * sizeof(value) + 1];
  char *ptr = buf + sizeof(buf);
  *--ptr = '\0';
  do {
    const unsigned digit = value % static_cast<unsigned long>(base);
    *--ptr = digit < 10 ? static_cast<char>('0' + digit) : static_cast<char>('A' + digit - 10);
    value /= static_cast<unsigned long>(base);
  } while (value);
  return write(ptr);
}
size_t HardwareSerial::print(unsigned long v, int base) { return printUnsigned(v, base); }
size_t HardwareSerial::print(long v, int base) {
  if (base == DEC && v < 0) {
    size_t n = write(static_cast<uint8_t>('-'));
    return n + printUnsigned(static_cast<unsigned long>(-(v + 1L)) + 1UL, base);
  }
  return printUnsigned(static_cast<unsigned long>(v), base);
}
size_t HardwareSerial::print(int v, int base) { return print(static_cast<long>(v), base); }
size_t HardwareSerial::print(unsigned int v, int base) { return print(static_cast<unsigned long>(v), base); }
size_t HardwareSerial::print(signed char v, int base) { return print(static_cast<long>(v), base); }
size_t HardwareSerial::print(unsigned char v, int base) { return print(static_cast<unsigned long>(v), base); }
size_t HardwareSerial::print(double value, int digits) {
  if (value != value) return print("nan");
  if (value > 4294967295.0 || value < -4294967295.0) return print("ovf");
  size_t n = 0;
  if (value < 0) { n += print('-'); value = -value; }
  if (digits < 0) digits = 0;
  if (digits > 9) digits = 9;
  double rounding = 0.5;
  for (int i = 0; i < digits; ++i) rounding /= 10.0;
  value += rounding;
  const unsigned long integer = static_cast<unsigned long>(value);
  n += print(integer);
  if (!digits) return n;
  n += print('.');
  double fraction = value - static_cast<double>(integer);
  while (digits--) {
    fraction *= 10.0;
    const unsigned digit = static_cast<unsigned>(fraction);
    n += write(static_cast<uint8_t>('0' + digit));
    fraction -= digit;
  }
  return n;
}
size_t HardwareSerial::println() { return write(static_cast<uint8_t>('\r')) + write(static_cast<uint8_t>('\n')); }
size_t HardwareSerial::println(const char *v) { return print(v) + println(); }
size_t HardwareSerial::println(char v) { return print(v) + println(); }
size_t HardwareSerial::println(signed char v, int b) { return print(v, b) + println(); }
size_t HardwareSerial::println(unsigned char v, int b) { return print(v, b) + println(); }
size_t HardwareSerial::println(int v, int b) { return print(v, b) + println(); }
size_t HardwareSerial::println(unsigned int v, int b) { return print(v, b) + println(); }
size_t HardwareSerial::println(long v, int b) { return print(v, b) + println(); }
size_t HardwareSerial::println(unsigned long v, int b) { return print(v, b) + println(); }
size_t HardwareSerial::println(double v, int d) { return print(v, d) + println(); }
