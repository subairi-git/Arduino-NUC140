// NUC140VE3CN UART0 pins: RX=PB0, TX=PB1, 8N1.
// To view output on a PC, use a compatible USB-to-TTL UART adapter:
// board PB1 (TX) -> adapter RX, board GND -> adapter GND.
// Match IO voltages; do NOT connect an RS-232 voltage-level port.
void setup() {
  Serial.begin(115200);
  Serial.println("NUC140 UART0 ready");
}
void loop() {
  Serial.print("uptime(ms): ");
  Serial.println(millis());
  delay(1000);
}
