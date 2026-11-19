/* SPI2 W25X16 Read Status Register-1 (0x05): no data modification. */
#include <SPI.h>

const uint8_t FLASH_CS = PD(0);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  Serial.println("NUC140 SPI2 W25X16 STATUS register (read-only)");
}
void loop() {
  SPI.beginTransaction(SPISettings(1000000UL, MSBFIRST, SPI_MODE0));
  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x05); // read status register-1
  uint8_t status = SPI.transfer(0x00);
  digitalWrite(FLASH_CS, HIGH);
  SPI.endTransaction();
  Serial.print("Status register 1: 0x");
  if (status < 16) Serial.print('0');
  Serial.println(status, HEX);
  if (SPI.lastError()) {
    Serial.print("SPI error = ");
    Serial.println(SPI.lastError());
  }
  delay(2000);
}
