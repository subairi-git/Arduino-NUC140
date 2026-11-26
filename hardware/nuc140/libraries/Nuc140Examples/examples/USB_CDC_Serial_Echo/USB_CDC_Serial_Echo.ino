#include <Arduino.h>
// Connect USB Device J3 to Windows; Nu-Link remains for programming/UART logs.
// USB CDC requires both authorized NUC140_USB_VID and NUC140_USB_PID.
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
    Serial.print(" dtr=");Serial.print(SerialUSB?1:0);
    Serial.print(" reset=");Serial.print(SerialUSB.debugResetCount());
    Serial.print(" setup=");Serial.print(SerialUSB.debugSetupCount());
    Serial.print(" rxBytes=");Serial.println(SerialUSB.debugBulkRxBytes());
    if(SerialUSB)SerialUSB.println("Hello from NUC140 USB CDC");
  }
}
