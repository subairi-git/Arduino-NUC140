#pragma once
#include <stddef.h>
#include <stdint.h>
#ifndef DEC
#define DEC 10
#endif
class USBSerial {
 public:
  USBSerial();
  bool begin(uint32_t baud=115200);
  void end();
  operator bool() const;
  bool connected() const;
  int available();
  int availableForWrite();
  int read();
  int peek();
  void flush();
  size_t write(uint8_t byte);
  size_t write(const uint8_t *buf,size_t len);
  size_t write(const char *str);
  size_t print(const char *str);
  size_t print(char v);
  size_t print(int v,int base=DEC);
  size_t print(unsigned int v,int base=DEC);
  size_t print(long v,int base=DEC);
  size_t print(unsigned long v,int base=DEC);
  size_t println();
  size_t println(const char *s);
  size_t println(int v,int base=DEC);
  size_t println(unsigned int v,int base=DEC);
  size_t println(long v,int base=DEC);
  size_t println(unsigned long v,int base=DEC);
  void poll();
 private:
  size_t printNumber(unsigned long v,int base);
};
extern USBSerial SerialUSB;
