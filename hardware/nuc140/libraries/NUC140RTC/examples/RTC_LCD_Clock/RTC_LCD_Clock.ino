/* Show NUC140 internal RTC on the board LCD 128x64.
 * Requires the previously hardware-tested NUC140LCD v0.1.1 library.
 * For setting the RTC first, use RTC_Serial_Clock example (S ... command).
 * This display example never resets the time.
 */
#include <NUC140RTC.h>
#include <NUC140LCD.h>
NUC140LCD lcd;
bool rtcOK = false;

void two(char *out, uint8_t n) {
  out[0] = static_cast<char>('0' + n / 10U);
  out[1] = static_cast<char>('0' + n % 10U);
}

void setup() {
  Serial.begin(115200);
  rtcOK=RTCClock.begin();
  if (!rtcOK) {
    Serial.print("RTC FAILED, error=");
    Serial.println(RTCClock.lastError());
  }
  if (!lcd.begin()) Serial.println("LCD INIT FAILED");
  lcd.clear();
  lcd.printText(7,0,"NUC140 RTC CLOCK");
  lcd.display();
}

void loop() {
  lcd.clear();
  lcd.rect(0,0,128,64);
  lcd.printText(10,5,"NUC140 RTC CLOCK");
  lcd.line(4,16,123,16);
  RTCDateTime dt;
  if (rtcOK && RTCClock.getDateTime(dt)) {
    char clockText[] = "00:00:00";
    two(clockText,dt.hour);
    two(clockText+3,dt.minute);
    two(clockText+6,dt.second);
    lcd.printText(35,23,clockText);
    char dateText[] = "00/00/0000";
    two(dateText,dt.day);
    two(dateText+3,dt.month);
    two(dateText+6,static_cast<uint8_t>(dt.year/100U));
    two(dateText+8,static_cast<uint8_t>(dt.year%100U));
    lcd.printText(28,41,dateText);
    Serial.println(clockText);
  } else {
    lcd.printText(4,27,"RTC NOT READY");
    lcd.printText(4,43,"SET VIA SERIAL");
  }
  lcd.display();
  delay(500);
}
