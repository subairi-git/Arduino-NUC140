#include <Arduino.h>
#include "NUC140RTC.h"

// Exact addresses from OpenNuvoton NUC100Series.h / StdDriver RTC.c.
#define RTC_REG32(a) (*reinterpret_cast<volatile uint32_t *>(a))
static const uint32_t SYS_PROTECT   = 0x50000100UL;
static const uint32_t CLK_PWRCON    = 0x50000200UL;
static const uint32_t CLK_APBCLK    = 0x50000208UL;
static const uint32_t CLK_CLKSTATUS = 0x5000020CUL;
static const uint32_t RTC_INIR      = 0x40008000UL;
static const uint32_t RTC_AER       = 0x40008004UL;
static const uint32_t RTC_TLR       = 0x4000800CUL;
static const uint32_t RTC_CLR       = 0x40008010UL;
static const uint32_t RTC_TSSR      = 0x40008014UL;
static const uint32_t RTC_DWR       = 0x40008018UL;

static const uint32_t RTC_INIT_KEY  = 0xA5EB1357UL;
static const uint32_t RTC_WRITE_KEY = 0xA965UL;
static const uint32_t RTC_AER_ENF   = 1UL << 16;
static const uint32_t LXT_ENABLE    = 1UL << 1;
static const uint32_t RTC_CLK_EN    = 1UL << 1;

static uint8_t bcd(uint8_t n) {
  return static_cast<uint8_t>(((n / 10U) << 4) | (n % 10U));
}
static uint8_t unbcd(uint8_t b) {
  return static_cast<uint8_t>((((b >> 4) & 0x0FU) * 10U) + (b & 0x0FU));
}
static bool isBcd(uint8_t b) {
  return (b & 0x0FU) <= 9U && ((b >> 4) & 0x0FU) <= 9U;
}
static bool leapYear(uint16_t y) {
  return (y % 4U == 0U) && ((y % 100U != 0U) || (y % 400U == 0U));
}
static bool valid(uint16_t y, uint8_t m, uint8_t d,
                  uint8_t h, uint8_t min, uint8_t s) {
  if (y < 2000 || y > 2099 || m < 1 || m > 12 || d < 1 ||
      h > 23 || min > 59 || s > 59) return false;
  static const uint8_t days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  uint8_t maxDay = days[m - 1];
  if (m == 2 && leapYear(y)) maxDay = 29;
  return d <= maxDay;
}
static uint8_t dayOfWeek(uint16_t y, uint8_t m, uint8_t d) {
  // Sakamoto: 0=Sunday, 4=Thursday for 2026-11-19.
  static const uint8_t monthOffset[] = {0,3,2,5,0,3,5,1,4,6,2,4};
  if (m < 3) --y;
  return static_cast<uint8_t>((y + y / 4U - y / 100U + y / 400U +
                               monthOffset[m - 1] + d) % 7U);
}

NUC140RTC RTCClock;
NUC140RTC::NUC140RTC() : beginDone_(false), error_(0) {}

bool NUC140RTC::begin() {
  if (beginDone_ && isRunning()) return true;
  error_ = 0;
  RTC_REG32(SYS_PROTECT) = 0x59UL;
  RTC_REG32(SYS_PROTECT) = 0x16UL;
  RTC_REG32(SYS_PROTECT) = 0x88UL;
  if ((RTC_REG32(SYS_PROTECT) & 1UL) == 0UL) {
    error_ = 1;
    return false;
  }
  RTC_REG32(CLK_PWRCON) |= LXT_ENABLE;
  RTC_REG32(CLK_APBCLK) |= RTC_CLK_EN;
  RTC_REG32(SYS_PROTECT) = 0UL;

  // LXT crystal startup can take several seconds; do not hang indefinitely.
  bool stable = false;
  for (uint16_t count = 0; count < 500U; ++count) {
    if (RTC_REG32(CLK_CLKSTATUS) & LXT_ENABLE) {
      stable = true;
      break;
    }
    delay(10);
  }
  if (!stable) { error_ = 2; return false; }

  // Preserve existing time if RTC already active, e.g. after MCU reset.
  if ((RTC_REG32(RTC_INIR) & 1UL) == 0UL) {
    RTC_REG32(RTC_INIR) = RTC_INIT_KEY;
    bool active = false;
    for (uint16_t i=0; i<100U; ++i) {
      if ((RTC_REG32(RTC_INIR) & 1UL) != 0UL) {
        active=true;
        break;
      }
      delay(1);
    }
    if (!active) { error_ = 3; return false; }
  }
  beginDone_ = true;
  return true;
}

bool NUC140RTC::isRunning() const {
  return ((RTC_REG32(CLK_CLKSTATUS) & LXT_ENABLE) != 0UL) &&
         ((RTC_REG32(RTC_INIR) & 1UL) != 0UL);
}

bool NUC140RTC::enableWrite() {
  if (!beginDone_) { error_ = 3; return false; }
  // Nuvoton RTC.c waits for AER write window to expire before reloading key.
  uint8_t n;
  for (n=0; n<50U && (RTC_REG32(RTC_AER) & RTC_AER_ENF); ++n) delay(1);
  if (RTC_REG32(RTC_AER) & RTC_AER_ENF) { error_ = 4; return false; }
  RTC_REG32(RTC_AER) = RTC_WRITE_KEY;
  for (n=0; n<50U && !(RTC_REG32(RTC_AER) & RTC_AER_ENF); ++n) delay(1);
  if ((RTC_REG32(RTC_AER) & RTC_AER_ENF) == 0UL) {
    error_ = 4;
    return false;
  }
  return true;
}

bool NUC140RTC::setDateTime(uint16_t year, uint8_t month, uint8_t day,
                            uint8_t hour, uint8_t minute, uint8_t second) {
  if (!valid(year,month,day,hour,minute,second)) { error_ = 5; return false; }
  if (!beginDone_ && !begin()) return false;
  error_ = 0;
  if (!enableWrite()) return false;
  // Two-digit RTC calendar year (2000..2099), BCD.
  uint32_t packedDate = (static_cast<uint32_t>(bcd(static_cast<uint8_t>(year - 2000U))) << 16) |
                         (static_cast<uint32_t>(bcd(month)) << 8) | bcd(day);
  uint32_t packedTime = (static_cast<uint32_t>(bcd(hour)) << 16) |
                         (static_cast<uint32_t>(bcd(minute)) << 8) | bcd(second);
  // 24-hour mode = bit 0 set; week day = Sunday 0..Saturday 6.
  RTC_REG32(RTC_TSSR) |= 1UL;
  RTC_REG32(RTC_DWR) = dayOfWeek(year, month, day);
  RTC_REG32(RTC_CLR) = packedDate;
  RTC_REG32(RTC_TLR) = packedTime;
  return true;
}

bool NUC140RTC::getDateTime(RTCDateTime &value) {
  if (!beginDone_ && !begin()) return false;
  error_ = 0;
  // Retry if seconds rollover during multi-register read.
  uint32_t rawTime=0, rawDate=0;
  bool consistent=false;
  for (uint8_t n=0; n<5U; ++n) {
    const uint32_t before = RTC_REG32(RTC_TLR);
    rawDate = RTC_REG32(RTC_CLR);
    rawTime = RTC_REG32(RTC_TLR);
    if (before == rawTime) { consistent=true; break; }
  }
  if (!consistent) { error_ = 6; return false; }
  const uint8_t yy = static_cast<uint8_t>((rawDate >> 16) & 0xFFUL);
  const uint8_t mo = static_cast<uint8_t>((rawDate >> 8) & 0xFFUL);
  const uint8_t da = static_cast<uint8_t>(rawDate & 0xFFUL);
  const uint8_t hh = static_cast<uint8_t>((rawTime >> 16) & 0xFFUL);
  const uint8_t mi = static_cast<uint8_t>((rawTime >> 8) & 0xFFUL);
  const uint8_t ss = static_cast<uint8_t>(rawTime & 0xFFUL);
  if (!isBcd(yy) || !isBcd(mo) || !isBcd(da) ||
      !isBcd(hh) || !isBcd(mi) || !isBcd(ss)) { error_ = 6; return false; }
  const uint16_t year = 2000 + unbcd(yy);
  const uint8_t month=unbcd(mo), day=unbcd(da), hour=unbcd(hh);
  const uint8_t minute=unbcd(mi), second=unbcd(ss);
  if (!valid(year,month,day,hour,minute,second)) { error_ = 6; return false; }
  value.year=year; value.month=month; value.day=day;
  value.hour=hour; value.minute=minute; value.second=second;
  value.dayOfWeek=static_cast<uint8_t>(RTC_REG32(RTC_DWR) & 7UL);
  return true;
}
