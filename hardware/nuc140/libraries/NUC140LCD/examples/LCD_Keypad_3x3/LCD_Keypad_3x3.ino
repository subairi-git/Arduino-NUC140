/*
 * Nu-LB-NUC140 V2.0 (NUC140VE3CN)
 * Keypad 3x3 bawaan -> LCD grafis 128x64 bawaan (SPI3).
 * Untuk Arduino NUC140 Core v0.1.4 + library NUC140LCD.
 *
 * Pemetaan sesuai Nuvoton Scankey.c:
 *   scan kolom PA2 -> PA3=1, PA4=4, PA5=7
 *   scan kolom PA1 -> PA3=2, PA4=5, PA5=8
 *   scan kolom PA0 -> PA3=3, PA4=6, PA5=9
 * Seluruh PA0..PA5 harus mode QUASI (Arduino INPUT_PULLUP).
 * LCD SPI3 menggunakan PD8..PD11, backlight PD14.
 *
 * Hasil: tombol terakhir, jumlah tekan, dan riwayat 12 tombol.
 * Setiap tekanan dicatat satu kali (debounce 30 ms + release).
 */
#include <Arduino.h>
#include <NUC140LCD.h>

NUC140LCD lcd;

const uint32_t COL[3] = { PA(2), PA(1), PA(0) };
const uint32_t ROW[3] = { PA(3), PA(4), PA(5) };
const uint8_t KEYMAP[3][3] = {
  {1, 4, 7},
  {2, 5, 8},
  {3, 6, 9}
};

const uint32_t DEBOUNCE_MS = 30;
uint8_t candidate = 0;
uint8_t stableKey = 0;
uint32_t changedAt = 0;
uint32_t pressCount = 0;
char lastKey = '-';
char history[13] = "";
uint8_t historyLen = 0;
bool lcdReady = false;

// Scan satu tombol. Prioritas tombol pertama jika dua tombol ditekan.
uint8_t scanKeypad() {
  uint8_t found = 0;
  for (uint8_t c = 0; c < 3; c++) digitalWrite(COL[c], HIGH);

  for (uint8_t c = 0; c < 3 && found == 0; c++) {
    digitalWrite(COL[c], LOW);
    delayMicroseconds(10);

    for (uint8_t r = 0; r < 3; r++) {
      if (digitalRead(ROW[r]) == LOW) {
        found = KEYMAP[c][r];
        break;
      }
    }
    digitalWrite(COL[c], HIGH);
  }
  return found;
}

void formatCount(uint32_t value, char *out) {
  char reverse[11];
  uint8_t length = 0;
  do {
    reverse[length++] = (char)('0' + value % 10UL);
    value /= 10UL;
  } while (value != 0 && length < 10);

  for (uint8_t i = 0; i < length; i++) {
    out[i] = reverse[length - 1 - i];
  }
  out[length] = '\0';
}

void renderLCD() {
  if (!lcdReady) return;

  char keyText[2] = { lastKey, '\0' };
  char countText[12];
  formatCount(pressCount, countText);

  lcd.clear();
  lcd.rect(0, 0, 128, 64);
  lcd.printText(8, 4, "NUC140 KEYPAD");
  lcd.line(5, 16, 122, 16);
  lcd.printText(8, 22, "TOMBOL:");
  lcd.printText(79, 22, keyText);
  lcd.printText(8, 35, "JUMLAH:");
  lcd.printText(79, 35, countText);
  lcd.printText(8, 49, "URUT:");
  lcd.printText(44, 49, history);

  if (!lcd.display()) {
    Serial.print("LCD display error=");
    Serial.println(lcd.lastError());
  }
}

void recordKey(uint8_t key) {
  if (key < 1 || key > 9) return;
  lastKey = (char)('0' + key);
  pressCount++;

  if (historyLen >= 12) {
    for (uint8_t i = 1; i < 12; i++) history[i - 1] = history[i];
    historyLen = 11;
  }
  history[historyLen++] = lastKey;
  history[historyLen] = '\0';

  Serial.print("Tombol = ");
  Serial.print(lastKey);
  Serial.print(" | Jumlah = ");
  Serial.println(pressCount);
  renderLCD();
}

void setup() {
  Serial.begin(115200);
  Serial.println("NUC140: Keypad 3x3 -> LCD 128x64");

  // INPUT_PULLUP dipetakan ke mode QUASI pada core NUC140.
  for (uint8_t pin = 0; pin < 6; pin++) {
    pinMode(PA(pin), INPUT_PULLUP);
    digitalWrite(PA(pin), HIGH);
  }

  lcdReady = lcd.begin();
  if (!lcdReady) {
    Serial.print("LCD INIT FAILED, code=");
    Serial.println(lcd.lastError());
  } else {
    lcd.backlight(true);
    renderLCD();
  }
  Serial.println("Tekan tombol 1..9 pada keypad onboard.");
}

void loop() {
  uint8_t rawKey = scanKeypad();
  uint32_t now = millis();

  if (rawKey != candidate) {
    candidate = rawKey;
    changedAt = now;
  }

  if ((uint32_t)(now - changedAt) >= DEBOUNCE_MS && candidate != stableKey) {
    stableKey = candidate;
    if (stableKey != 0) recordKey(stableKey);
  }
  delay(2);
}
