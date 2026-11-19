/*
 * Nu-LB-NUC140 V2.0, NUC140VE3CN.
 * SPI2 Flash W25X16, lower chip U3: PD0 CS / PD1 CLK / PD2 MISO / PD3 MOSI.
 * Read JEDEC ID (0x9F), read-only (no erase/write).
 */
#include <SPI.h>

const uint8_t FLASH_CS = PD(0);
SPISettings flashSettings(1000000UL, MSBFIRST, SPI_MODE0);

void printHexByte(uint8_t value) {
  if (value < 16U) Serial.print('0');
  Serial.print(value, HEX);
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  Serial.println("NUC140 SPI2 / W25X16 JEDEC ID TEST");
  Serial.println("CS=PD0 SCK=PD1 MISO=PD2 MOSI=PD3");
  Serial.println("Flash read-only; no write or erase");
  if (!SPI.isInitialized()) {
    Serial.print("SPI INIT FAILED, error=");
    Serial.println(SPI.lastError());
  }
}

void loop() {
  if (!SPI.isInitialized()) {
    Serial.println("ERROR: SPI2 not initialized");
    delay(1000);
    return;
  }

  SPI.beginTransaction(flashSettings);
  digitalWrite(FLASH_CS, LOW);
  delayMicroseconds(5);

  (void)SPI.transfer(0x9F); // Read JEDEC ID
  uint8_t manufacturer = SPI.transfer(0x00);
  uint8_t memoryType   = SPI.transfer(0x00);
  uint8_t capacity     = SPI.transfer(0x00);

  digitalWrite(FLASH_CS, HIGH);
  SPI.endTransaction();

  Serial.print("JEDEC ID: ");
  printHexByte(manufacturer);
  Serial.print(' ');
  printHexByte(memoryType);
  Serial.print(' ');
  printHexByte(capacity);
  Serial.println();

  if (SPI.lastError()) {
    Serial.print("SPI ERROR CODE: ");
    Serial.println(SPI.lastError());
  } else if ((manufacturer == 0x00 && memoryType == 0x00 && capacity == 0x00) ||
             (manufacturer == 0xFF && memoryType == 0xFF && capacity == 0xFF)) {
    Serial.println("SPI INVALID RESPONSE: check chip, CS and pin mapping");
  } else {
    Serial.println("SPI2 JEDEC READ PASS (check manufacturer/capacity)");
  }
  delay(2000);
}
