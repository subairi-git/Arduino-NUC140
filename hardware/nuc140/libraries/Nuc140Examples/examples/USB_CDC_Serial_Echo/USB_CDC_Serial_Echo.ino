#include <Arduino.h>
// Connect USB Device J3 to Windows; Nu-Link remains for programming/UART logs.
// USB CDC test VID:PID 0416:B002 belongs to Nuvoton and is for laboratory use only.
void setup() {
  Serial.begin(115200);
  Serial.println("NUC140 USB CDC ACM TEST");
  bool started=SerialUSB.begin(115200);
  Serial.println(started ? "USB CDC engine started" : "USB CDC clock start FAILED");
}
void loop() {
  while(SerialUSB.available()) {
    int c=SerialUSB.read();
    if(c>=0) SerialUSB.write((uint8_t)c); // echo back to Windows
  }
  static uint32_t last=0;
  if(millis()-last>=1000UL) {
    last=millis();
    Serial.print("millis=");Serial.print(last);
    Serial.print(" usbConfigured=");Serial.print(SerialUSB.connected()?1:0);
    Serial.print(" dtr=");Serial.println(SerialUSB?1:0);
    if(SerialUSB)SerialUSB.println("Hello from NUC140 USB CDC");
  }
}
