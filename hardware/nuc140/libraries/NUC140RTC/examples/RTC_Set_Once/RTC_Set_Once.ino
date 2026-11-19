/* One-shot RTC setting and progress check.
 * IMPORTANT: SET_TIME_ON_BOOT=true always resets the clock when sketch starts.
 * After verifying ticking, change to false and re-upload to test persistence.
 */
#include <NUC140RTC.h>
const bool SET_TIME_ON_BOOT = true;

void setup() {
  Serial.begin(115200);
  Serial.println("RTC SET ONCE / READ EVERY SECOND");
  if (!RTCClock.begin()) {
    Serial.print("RTC INIT FAILED error=");
    Serial.println(RTCClock.lastError());
    return;
  }
  if (SET_TIME_ON_BOOT) {
    // Date and time are explicitly illustrative; adjust to current WIB.
    if (RTCClock.setDateTime(2026, 11, 19, 15, 30, 0)) {
      Serial.println("RTC SET OK (2026-11-19 15:30:00)");
    } else {
      Serial.print("RTC SET ERROR=");
      Serial.println(RTCClock.lastError());
    }
  }
}

void loop() {
  RTCDateTime t;
  if (RTCClock.getDateTime(t)) {
    Serial.print(t.year); Serial.print('/'); Serial.print(t.month);
    Serial.print('/'); Serial.print(t.day); Serial.print(' ');
    Serial.print(t.hour); Serial.print(':'); Serial.print(t.minute);
    Serial.print(':'); Serial.println(t.second);
  } else {
    Serial.print("RTC READ ERROR="); Serial.println(RTCClock.lastError());
  }
  delay(1000);
}
