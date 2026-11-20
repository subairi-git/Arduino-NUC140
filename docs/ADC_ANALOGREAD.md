# Arduino analogRead() for Nu-LB-NUC140 V2.0 (experimental)

**Verified previously on real hardware:** the pot VR1 on PA7/ADC7 works in the
standalone register-based sketch; UART0 TX/RX also work. This new core API
must still be verified with a hardware upload. The CI checks cross compilation,
not hardware signals.

## Basic usage

\`\`\`cpp
void setup() { Serial.begin(115200); }
void loop() {
  int value = analogRead(PA(7)); // VR1, ADC7, 0..4095
  Serial.println(value);
  delay(200);
}
\`\`\`

Implementation:
- \`Arduino.h\`: declares \`int analogRead(uint32_t pin)\`
- \`wiring_analog.c\`: initializes ADC peripheral on first call and configures PA0..PA7
- 12-bit results (0..4095), returns **-1** if pin is unsupported or ADC
  clock initialization/conversion times out (experimental extension)
- Uses HIRC 22.1184 MHz divided by 7 for ADC module clock
- Uses polling, not ADC interrupts; it does not modify UART or SysTick

Supported inputs are \`PA(0)\` through \`PA(7)\`. VR1 is on **PA(7)**.
Using PA0..PA7 for ADC changes their pin multiplexing, including disabling
conflicting EBI/smart-card alternative functions. Do not use those pins for
other peripherals simultaneously. Analog reference is governed by the board
hardware; do not assume a precise voltage reference without measurement.

## Manually update a Boards Manager 0.1.2 installation

First close Arduino IDE. Keep the working Nu-Link upload hotfix and UART
files. Only copy these two files from the patch or repository:

1. \`hardware/nuc140/cores/arduino/Arduino.h\` (back up the previous header)
2. \`hardware/nuc140/cores/arduino/wiring_analog.c\` (new file)

Into the installed core folder:

\`%LOCALAPPDATA%\Arduino15\packages\nuc140\hardware\nuc140\0.1.2\cores\arduino\`

Reopen the IDE, compile, upload and open the UART adapter's Serial Monitor
at 115200. The repo's development branch and a manual patch are **not**
automatically distributed by Arduino Boards Manager; that requires a
separate tagged release.

Official register source: https://github.com/OpenNuvoton/NUC100BSP
(NUC100Series.h, StdDriver ADC, sys.h).
