#pragma once
#include <stdint.h>

/* Experimental 128x64 LCD for Nu-LB-NUC140 V2.0, SPI3 9-bit.
   PD8=CS (GPIO), PD9=SCK, PD11=MOSI, PD14=backlight (active low).
   No changes to SPI2 flash or Wire I2C1. */
class NUC140LCD {
public:
  enum { WIDTH=128, HEIGHT=64 };
  NUC140LCD();
  bool begin();
  void backlight(bool on);
  void setContrast(uint8_t contrast);
  void clear();
  void fill(bool on);
  void pixel(int16_t x, int16_t y, bool on=true);
  void line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool on=true);
  void rect(int16_t x, int16_t y, int16_t w, int16_t h, bool on=true);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool on=true);
  void drawChar(int16_t x, int16_t y, char ch, bool on=true);
  void printText(int16_t x, int16_t y, const char* text, bool on=true);
  void printLine(uint8_t row, const char* text);
  bool display(); // flush 1024-byte framebuffer to LCD
  uint8_t lastError() const { return last_error_; }
private:
  uint8_t buffer_[WIDTH * (HEIGHT/8)];
  bool initialized_;
  uint8_t last_error_;
  bool send9(uint16_t data);
  bool cmd(uint8_t command);
  void setAddress(uint8_t page, uint8_t column);
};
