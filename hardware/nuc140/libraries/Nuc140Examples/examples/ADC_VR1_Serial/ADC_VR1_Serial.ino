/*
 * Nu-LB-NUC140 V2.0 built-in potentiometer VR1 -> PA7 / ADC7.
 * Native ADC resolution is 12 bits (0..4095).
 * UART0: PB1(TX), PB0(RX), 115200 8N1.
 *
 * Requires experimental NUC140 core with analogRead(PA(n)).
 */
void setup() {
  Serial.begin(115200);
  Serial.println("NUC140 / VR1 / ADC7 analogRead()");
}
void loop() {
  int value = analogRead(PA(7));
  if (value < 0) {
    Serial.println("ADC ERROR (clock/pin/conversion timeout)");
  } else {
    Serial.print("ADC7=");
    Serial.print(value);
    Serial.print(" / 4095  persen=");
    Serial.print((unsigned long)value * 100UL / 4095UL);
    Serial.println("%");
  }
  delay(200);
}
