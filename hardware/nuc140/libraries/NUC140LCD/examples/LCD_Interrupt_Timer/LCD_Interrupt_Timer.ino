/*
 * Integrated GPIO Interrupt PB15 + TIMER0 1Hz + LCD 128x64 SPI3.
 * Tested peripheral APIs, new IRQ/timer drivers awaiting hardware test.
 *
 * LCD shows seconds since boot and SW INT button count.
 * The ISR callbacks only update volatile variables, NEVER call Serial/LCD.
 */
#include <Arduino.h>
#include <NUC140LCD.h>

NUC140LCD lcd;
volatile uint32_t elapsedSeconds = 0;
volatile uint32_t keyCount = 0;
volatile uint32_t lastKeyMs = 0;
bool lcdReady = false;

void onTimerSecond() {
  elapsedSeconds++;
}

void onSwitchPress() {
  uint32_t now = millis();
  if (keyCount == 0 || (uint32_t)(now - lastKeyMs) >= 180UL) {
    lastKeyMs = now;
    keyCount++;
  }
}

void formatNumber(uint32_t number, char* dst) {
  char backwards[11];
  uint8_t len = 0;
  do {
    backwards[len++] = (char)('0' + (number % 10UL));
    number /= 10UL;
  } while (number && len < 10U);
  for (uint8_t i = 0; i < len; ++i) dst[i] = backwards[len - i - 1];
  dst[len] = '\0';
}

void render(uint32_t seconds, uint32_t presses) {
  if (!lcdReady) return;
  char secText[12], pressText[12];
  formatNumber(seconds, secText);
  formatNumber(presses, pressText);
  lcd.clear();
  lcd.rect(0, 0, 128, 64);
  lcd.printText(6, 4, "GPIO IRQ + TIMER");
  lcd.line(4, 17, 123, 17);
  lcd.printText(6, 23, "DETIK:");
  lcd.printText(68, 23, secText);
  lcd.printText(6, 37, "SW PB15:");
  lcd.printText(68, 37, pressText);
  lcd.printText(6, 51, "PC12: TICK");
  if (!lcd.display()) {
    Serial.print("LCD refresh error: ");
    Serial.println(lcd.lastError());
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PC(12), OUTPUT);
  digitalWrite(PC(12), HIGH);
  pinMode(PB(15), INPUT_PULLUP);

  lcdReady = lcd.begin();
  if (!lcdReady) {
    Serial.print("LCD init failed: ");
    Serial.println(lcd.lastError());
  } else {
    lcd.backlight(true);
    render(0, 0);
  }

  attachInterrupt(digitalPinToInterrupt(PB(15)), onSwitchPress, FALLING);
  if (!timerBegin(0, 1000000UL, onTimerSecond)) {
    Serial.println("ERROR: TIMER0 did not start");
  } else {
    Serial.println("TIMER0 1-second periodic IRQ enabled");
  }
  Serial.println("Press SW INT PB15, observe LCD seconds / button presses.");
}

void loop() {
  static uint32_t lastSeconds = 0xFFFFFFFFUL;
  static uint32_t lastPresses = 0xFFFFFFFFUL;
  uint32_t seconds = elapsedSeconds;
  uint32_t presses = keyCount;

  if (seconds != lastSeconds || presses != lastPresses) {
    if (seconds != lastSeconds) digitalWrite(PC(12), (seconds & 1UL) ? LOW : HIGH);
    render(seconds, presses);
    Serial.print("TIMER0 seconds=");
    Serial.print(seconds);
    Serial.print(" | SW_INT count=");
    Serial.println(presses);
    lastSeconds = seconds;
    lastPresses = presses;
  }
}
