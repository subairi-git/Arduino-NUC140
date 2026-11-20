/* Nu-LB-NUC140 V2.0 I2C1 scanner (PA10 SDA, PA11 SCL). */
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(100000);
  Serial.println("NUC140 I2C1 scanner - EEPROM 24LC64 expected at 0x50");
}

void loop() {
  unsigned int found = 0;
  Serial.println("Scanning I2C1...");
  for (uint8_t addr = 0x08; addr < 0x78; ++addr) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C ACK at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      ++found;
    }
  }
  Serial.print("Devices found: ");
  Serial.println(found);
  delay(3000);
}
