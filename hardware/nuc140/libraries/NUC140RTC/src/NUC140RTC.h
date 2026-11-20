#pragma once
#include <stdint.h>

/* Nu-LB-NUC140 V2.0 on-chip RTC + board 32.768 kHz crystal X1.
 * DateTime is Gregorian (2000..2099), 24-hour time, WIB only if user sets WIB.
 * RTC runs independently from millis(). It does not automatically sync to PC.
 */
struct RTCDateTime {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t dayOfWeek;  // 0 = Sunday, 1 = Monday, ... 6 = Saturday
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

class NUC140RTC {
public:
  NUC140RTC();
  // Enable LXT and RTC; preserve previously running date and time.
  bool begin();
  // Set time explicitly. No automatic reset at begin()/MCU reset.
  bool setDateTime(uint16_t year, uint8_t month, uint8_t day,
                   uint8_t hour, uint8_t minute, uint8_t second);
  bool getDateTime(RTCDateTime &value);
  bool isRunning() const;
  uint8_t lastError() const { return error_; }
  // 0=OK, 1=SYS unlock, 2=LXT timeout, 3=RTC init timeout,
  // 4=access key timeout, 5=invalid input date/time, 6=invalid RTC date.
private:
  bool beginDone_;
  uint8_t error_;
  bool enableWrite();
};

extern NUC140RTC RTCClock;
