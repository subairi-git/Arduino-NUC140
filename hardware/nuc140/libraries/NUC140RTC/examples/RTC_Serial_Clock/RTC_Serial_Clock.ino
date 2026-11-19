/* Nu-LB-NUC140 V2.0 built-in RTC clock on X1 (32.768kHz).
 * Serial Monitor: 115200 baud, Newline.
 * Command to set date/time: S 2026-11-19 15:31:00
 * MCU Reset will NOT reset RTC time while RTC still powered.
 */
#include <NUC140RTC.h>

char command[36];
uint8_t used = 0;

void print02(uint8_t value) {
  if (value < 10U) Serial.print('0');
  Serial.print(value);
}

bool digit(char c) { return c >= '0' && c <= '9'; }
uint8_t two(const char *p) {
  return static_cast<uint8_t>((p[0]-'0')*10 + p[1]-'0');
}
uint16_t four(const char *p) {
  return static_cast<uint16_t>((p[0]-'0')*1000 + (p[1]-'0')*100 +
                               (p[2]-'0')*10 + p[3]-'0');
}
void parseCommand() {
  // Exact command: S YYYY-MM-DD HH:MM:SS (21 chars)
  bool ok = used == 21U && (command[0] == 'S' || command[0] == 's') &&
            command[1] == ' ' && command[6] == '-' && command[9] == '-' &&
            command[12] == ' ' && command[15] == ':' && command[18] == ':';
  const uint8_t indices[] = {2,3,4,5,7,8,10,11,13,14,16,17,19,20};
  if (ok) for (unsigned int i=0; i<sizeof(indices); ++i) {
    if (!digit(command[indices[i]])) { ok=false; break; }
  }
  if (ok) {
    ok = RTCClock.setDateTime(four(command+2), two(command+7), two(command+10),
                              two(command+13), two(command+16), two(command+19));
    if (ok) Serial.println("RTC SET OK");
    else {
      Serial.print("RTC SET FAILED error=");
      Serial.println(RTCClock.lastError());
    }
  } else {
    Serial.println("Format: S YYYY-MM-DD HH:MM:SS");
    Serial.println("Example: S 2026-11-19 15:31:00");
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("NUC140 RTC INTERNAL CLOCK TEST v0.1.0");
  Serial.println("Clock source: board 32.768 kHz crystal (LXT)");
  if (!RTCClock.begin()) {
    Serial.print("RTC INIT ERROR=");
    Serial.println(RTCClock.lastError());
    Serial.println("Error 2: 32kHz crystal not stable; check X1/VBAT");
    return;
  }
  Serial.println("RTC initialized. Existing date/time preserved.");
  Serial.println("To set clock: S YYYY-MM-DD HH:MM:SS");
  Serial.println("Use Serial Monitor 115200 baud + Newline");
}

void loop() {
  while (Serial.available()) {
    const int c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (used) parseCommand();
      used=0;
    } else if (c >= 0 && used < sizeof(command)-1U) {
      command[used++] = static_cast<char>(c);
      command[used] = '\0';
    }
  }

  static uint32_t lastPrint = 0;
  if ((uint32_t)(millis()-lastPrint) >= 1000UL) {
    lastPrint = millis();
    RTCDateTime now;
    if (!RTCClock.getDateTime(now)) {
      Serial.print("RTC DATA NOT SET / ERROR=");
      Serial.println(RTCClock.lastError());
      return;
    }
    Serial.print(now.year); Serial.print('-'); print02(now.month);
    Serial.print('-'); print02(now.day);
    Serial.print(' '); print02(now.hour);
    Serial.print(':'); print02(now.minute);
    Serial.print(':'); print02(now.second);
    Serial.print(" WDAY="); Serial.println(now.dayOfWeek);
  }
}
