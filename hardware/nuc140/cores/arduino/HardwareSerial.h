#pragma once
#include <stddef.h>
#include <stdint.h>
// NUC140VE3CN UART0 on PB0(RXD)/PB1(TXD), 8N1, polling.
// Initial adapter supports common Arduino Serial calls, not UART IRQs.
#ifndef DEC
#define DEC 10
#endif
#ifndef HEX
#define HEX 16
#endif
#ifndef OCT
#define OCT 8
#endif
#ifndef BIN
#define BIN 2
#endif

class HardwareSerial {
public:
  HardwareSerial();
  void begin(uint32_t baud);
  void end();
  operator bool() const { return started_; }
  int available();
  int availableForWrite();
  int peek();
  int read();
  void flush();
  size_t write(uint8_t byte);
  size_t write(const uint8_t *buffer, size_t length);
  size_t write(const char *str);
  size_t print(const char *str);
  size_t print(char value);
  size_t print(signed char value, int base = DEC);
  size_t print(unsigned char value, int base = DEC);
  size_t print(int value, int base = DEC);
  size_t print(unsigned int value, int base = DEC);
  size_t print(long value, int base = DEC);
  size_t print(unsigned long value, int base = DEC);
  size_t print(double value, int digits = 2);
  size_t println();
  size_t println(const char *str);
  size_t println(char value);
  size_t println(signed char value, int base = DEC);
  size_t println(unsigned char value, int base = DEC);
  size_t println(int value, int base = DEC);
  size_t println(unsigned int value, int base = DEC);
  size_t println(long value, int base = DEC);
  size_t println(unsigned long value, int base = DEC);
  size_t println(double value, int digits = 2);
private:
  size_t printUnsigned(unsigned long value, int base);
  bool started_;
  int peeked_;
};
extern HardwareSerial Serial;
