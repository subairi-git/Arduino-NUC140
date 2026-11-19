#include <Arduino.h>
#include "NUC140LCD.h"
#include "Font5x7.h"

#define LCD_REG(address) (*reinterpret_cast<volatile uint32_t*>(address))

// NUC100/140 registers; SPI3 is separate from SPI2 Flash.
static const uint32_t SYS_PROTECT = 0x50000100UL;
static const uint32_t GPD_MFP     = 0x5000003CUL;
static const uint32_t CLK_APBCLK  = 0x50000208UL;
static const uint32_t CLK_CLKSEL1 = 0x50000214UL;
static const uint32_t SPI3_CNTRL  = 0x40134000UL;
static const uint32_t SPI3_DIV    = 0x40134004UL;
static const uint32_t SPI3_SSR    = 0x40134008UL;
static const uint32_t SPI3_TX0    = 0x40134020UL;
static const uint32_t SPI3_CNTRL2 = 0x4013403CUL;
static const uint32_t SPI_BUSY    = 1UL;
static const uint32_t WAIT_LIMIT  = 1000000UL;

NUC140LCD::NUC140LCD(): initialized_(false), last_error_(0) { clear(); }

void NUC140LCD::clear() {
  for (unsigned i=0; i<sizeof(buffer_); i++) buffer_[i]=0;
}
void NUC140LCD::fill(bool on) {
  for (unsigned i=0; i<sizeof(buffer_); i++) buffer_[i]=on ? 0xFF : 0;
}
void NUC140LCD::backlight(bool on) {
  pinMode(PD(14), OUTPUT);
  digitalWrite(PD(14), on ? LOW : HIGH); // verified active-low by board docs
}

bool NUC140LCD::send9(uint16_t data) {
  if (!initialized_) { last_error_=1; return false; }
  // Use hardware SPI3 SS0, as in Nuvoton Nu-LB-NUC140 LCD.c.
  // SS0 low while SSR bit0=1 (manual, active low).
  LCD_REG(SPI3_SSR)=1UL;
  uint32_t timeout=WAIT_LIMIT;
  while ((LCD_REG(SPI3_CNTRL)&SPI_BUSY) && timeout) --timeout;
  if (!timeout) { LCD_REG(SPI3_SSR)=0UL; last_error_=2; return false; }
  LCD_REG(SPI3_TX0)=data & 0x1FFUL; // bit8: 0 command, 1 data
  LCD_REG(SPI3_CNTRL)|=SPI_BUSY;
  timeout=WAIT_LIMIT;
  while ((LCD_REG(SPI3_CNTRL)&SPI_BUSY) && timeout) --timeout;
  LCD_REG(SPI3_SSR)=0UL;
  if (!timeout) { last_error_=3; return false; }
  return true;
}
bool NUC140LCD::cmd(uint8_t command) { return send9(command); }
void NUC140LCD::setAddress(uint8_t page, uint8_t column) {
  (void)cmd((uint8_t)(0xB0U | (page & 0x0FU)));
  (void)cmd((uint8_t)(0x10U | ((column >> 4) & 0x0FU)));
  (void)cmd((uint8_t)(column & 0x0FU));
}

bool NUC140LCD::begin() {
  if (initialized_) return true;
  last_error_=0;
  // Power/backlight stay off until controller setup is complete.
  backlight(false);
  // SPI3 hardware SS0 is on PD8; do not configure PD8 as GPIO.

  LCD_REG(SYS_PROTECT)=0x59UL;
  LCD_REG(SYS_PROTECT)=0x16UL;
  LCD_REG(SYS_PROTECT)=0x88UL;
  if (!(LCD_REG(SYS_PROTECT) & 1UL)) { last_error_=4; return false; }
  LCD_REG(CLK_APBCLK)|=(1UL<<15); // SPI3 clock
  LCD_REG(CLK_CLKSEL1)|=(1UL<<7); // SPI3 source HCLK (22.1184 MHz)
  // Nuvoton reference: PD8 = SPI3_SS0, PD9 = SPI3_CLK, PD11 = MOSI0.
  LCD_REG(GPD_MFP) |= (1UL<<8) | (1UL<<9) | (1UL<<11);
  LCD_REG(SYS_PROTECT)=0;
  LCD_REG(SPI3_SSR)=0UL; // disable automatic hardware SS
  LCD_REG(SPI3_CNTRL2)=(1UL<<31); // BCn=1; SPI clock=HCLK/(DIV+1)
  LCD_REG(SPI3_DIV)=21UL; // 22.1184 MHz /22 = ~1.005 MHz
  // SPI3, 9-bit, MSB first, master, type1 (TX_NEG=1, CPOL=0)
  LCD_REG(SPI3_CNTRL)=(9UL<<3) | (1UL<<2);
  initialized_=true;
  delay(50);

  // The board-specific Nuvoton LCD.c (Nu-LB-NUC140 BSP) initializes
  // this LCD with EB, 81, A0, C0, AF. The previous speculative
  // ST7565 sequence E2/AE/A2/C8/2F/81/18/AF could leave it blank.
  const uint8_t initSeq[] = {
    0xEB, 0x81, 0xA0, 0xC0, 0xAF
  };
  for (unsigned i=0; i<sizeof(initSeq); i++) if(!cmd(initSeq[i])) return false;
  delay(40);
  clear();
  if(!display()) return false;
  backlight(true);
  return true;
}
void NUC140LCD::setContrast(uint8_t contrast) {
  if (contrast > 63U) contrast=63U;
  if (initialized_) { (void)cmd(0x81); (void)cmd(contrast); }
}

bool NUC140LCD::display() {
  if (!initialized_) { last_error_=1; return false; }
  last_error_=0;
  // Reference BSP maps logical x -> physical 129-x in 132-column RAM.
  // Start at 2 and stream buffer from right-to-left per page.
  for (uint8_t page=0; page<8; ++page) {
    setAddress(page, 2);
    if (last_error_) return false;
    for (int16_t x=127; x>=0; --x) {
      if (!send9((uint16_t)(0x100UL | buffer_[(uint16_t)page*128U + (uint16_t)x]))) return false;
    }
  }
  return true;
}

void NUC140LCD::pixel(int16_t x,int16_t y,bool on) {
  if (x<0 || x>=WIDTH || y<0 || y>=HEIGHT) return;
  const uint16_t index=(uint16_t)x+((uint16_t)y>>3U)*128U;
  const uint8_t mask=(uint8_t)(1U<<(y&7));
  if (on) buffer_[index]|=mask;
  else buffer_[index]&=(uint8_t)~mask;
}
void NUC140LCD::line(int16_t x0,int16_t y0,int16_t x1,int16_t y1,bool on) {
  int16_t dx=(x0>x1)?(x0-x1):(x1-x0), sx=(x0<x1)?1:-1;
  int16_t dy=(y0>y1)?(y1-y0):(y0-y1), sy=(y0<y1)?1:-1;
  int16_t error=dx+dy;
  for(;;) {
    pixel(x0,y0,on);
    if(x0==x1 && y0==y1) break;
    int16_t e2=(int16_t)(2*error);
    if(e2>=dy) { error+=dy; x0+=sx; }
    if(e2<=dx) { error+=dx; y0+=sy; }
  }
}
void NUC140LCD::rect(int16_t x,int16_t y,int16_t w,int16_t h,bool on) {
  if(w<=0||h<=0) return;
  line(x,y,x+w-1,y,on); line(x,y+h-1,x+w-1,y+h-1,on);
  line(x,y,x,y+h-1,on); line(x+w-1,y,x+w-1,y+h-1,on);
}
void NUC140LCD::fillRect(int16_t x,int16_t y,int16_t w,int16_t h,bool on) {
  if(w<=0||h<=0) return;
  for(int16_t yy=y; yy<y+h; ++yy) for(int16_t xx=x; xx<x+w; ++xx) pixel(xx,yy,on);
}
void NUC140LCD::drawChar(int16_t x,int16_t y,char ch,bool on) {
  uint8_t chv=(uint8_t)ch;
  if(chv>='a' && chv<='z') chv=(uint8_t)(chv-32);
  if(chv<32 || chv>126) chv='?';
  for(uint8_t col=0; col<5; ++col) {
    uint8_t bits=kLCD5x7[chv-32][col];
    for(uint8_t row=0; row<7; ++row) if(bits&(1U<<row)) pixel(x+col,y+row,on);
  }
}
void NUC140LCD::printText(int16_t x,int16_t y,const char *s,bool on) {
  if(!s) return;
  while(*s && x<WIDTH) { drawChar(x,y,*s++,on); x+=6; }
}
void NUC140LCD::printLine(uint8_t row,const char *s) {
  if(row>=8) return;
  // Clear only the target 8-pixel-high text line.
  fillRect(0,(int16_t)row*8,128,8,false);
  printText(0,(int16_t)row*8,s);
}
