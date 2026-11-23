#include <Arduino.h>

// Connect an external PASSIVE buzzer between PB10 and GND.
// The onboard PB11 buzzer is active (fixed pitch), so it cannot play scales.
const uint32_t SPEAKER = PB(10);

// Original short melody (frequencies in Hz)
const unsigned int notes[] = {262, 294, 330, 349, 392, 440, 392, 330, 294, 262};
const unsigned int lengths[] = {220, 220, 220, 220, 360, 220, 220, 220, 220, 500};

void setup() {
  Serial.begin(115200);
  Serial.println("NUC140 tone() passive buzzer test (PB10)");
}

void loop() {
  for(unsigned int i=0; i<sizeof(notes)/sizeof(notes[0]); ++i) {
    Serial.print("Note Hz: ");
    Serial.println(notes[i]);
    tone(SPEAKER,notes[i],lengths[i]);
    delay(lengths[i]+60);
    noTone(SPEAKER);
  }
  delay(1200);
}
