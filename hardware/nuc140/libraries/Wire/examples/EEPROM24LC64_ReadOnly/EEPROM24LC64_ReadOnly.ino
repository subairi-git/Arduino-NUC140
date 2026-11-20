/*
 * Nu-LB-NUC140 V2.0 onboard EEPROM 24LC64 on I2C1.
 * PA10 SDA / PA11 SCL; EEPROM address 0x50.
 * Read 16 bytes from EEPROM location 0x0000 without writing EEPROM.
 */
#include <Wire.h>

static const uint8_t EEPROM_ADDR = 0x50;
static const uint16_t READ_OFFSET = 0x0000;
static const size_t READ_LENGTH = 16;

void printHex(uint8_t value) {
  if (value < 16) Serial.print('0');
  Serial.print(value, HEX);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(100000);
  Serial.println("NUC140 Wire.h / 24LC64 read-only test");
}

void loop() {
  // Probe: START + address, read ACK, STOP.
  Wire.beginTransmission(EEPROM_ADDR);
  uint8_t status = Wire.endTransmission();
  if (status != 0) {
    Serial.print("EEPROM NOT FOUND, error=");
    Serial.print(status);
    Serial.print(" I2CSTATUS=0x");
    Serial.println(Wire.lastStatus(), HEX);
    delay(2000);
    return;
  }

  Serial.println("EEPROM FOUND (0x50), ACK OK");

  // Set internal EEPROM read pointer (2 address bytes).
  // endTransmission(false) avoids STOP: requestFrom issues REPEATED START.
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(static_cast<uint8_t>(READ_OFFSET >> 8));
  Wire.write(static_cast<uint8_t>(READ_OFFSET & 0xFFU));
  status = Wire.endTransmission(false);
  if (status != 0) {
    Serial.print("Address setup FAILED, error=");
    Serial.println(status);
    delay(2000);
    return;
  }

  size_t count = Wire.requestFrom(EEPROM_ADDR, READ_LENGTH);
  if (count != READ_LENGTH) {
    Serial.print("EEPROM READ FAILED, count=");
    Serial.print(static_cast<unsigned int>(count));
    Serial.print(" I2CSTATUS=0x");
    Serial.println(Wire.lastStatus(), HEX);
  } else {
    Serial.println("READ EEPROM (first 16 bytes):");
    while (Wire.available()) {
      int data = Wire.read();
      printHex(static_cast<uint8_t>(data));
      Serial.print(' ');
    }
    Serial.println();
    Serial.println("Wire I2C1 READ PASS");
  }
  delay(3000);
}
