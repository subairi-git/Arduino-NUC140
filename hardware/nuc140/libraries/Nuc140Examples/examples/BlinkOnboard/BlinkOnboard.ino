// Nu-LB-NUC140 V2.0: on-board LEDs PC12-PC15, active LOW.
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED off
}
void loop() {
  digitalWrite(LED_BUILTIN, LOW);  // LED on
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH); // LED off
  delay(500);
}
