# Arduino analogWrite() hardware PWM for NUC140VE3CN

**Status: Preview v0.1.1, hardware retest pending.** NUC140 PWM0..PWM3 output, intended first hardware test
with the Nu-LB-NUC140 V2.0 RGB LED. ARM Cortex-M0 Clang compile passed, physical PWM
response pending user test.

## Pin map and polarity

| Color (board RGB LED) | Pin | Peripheral | LED polarity |
|---|---|---|---|
| Red | PA14 | PWM2 | LOW = ON |
| Green | PA13 | PWM1 | LOW = ON |
| Blue | PA12 | PWM0 | LOW = ON |

Examples use \`rgbWrite(red,green,blue)\` with intuitive 0..255 brightness.
The new \`analogWrite(pin,value)\` is standard voltage-level semantics 0=LOW,
255=HIGH. For common-anode LED, use \`analogWrite(pin,255-brightness)\`.

## PWM configuration

- PWMA clock source HCLK assumed 22.1184 MHz.
- CP01=CP23=1 => /2; CSR channels 0..3 = 0 => /2.
- CNR=5529 (5530 counts) => ~999.9277 Hz.
- For values 1..254, PWM CMR=(value*5530+127)/255 -1.
- For value 0 or 255, pin is switched to GPIO and driven LOW or HIGH
  respectively for exact endpoints; next intermediate \`analogWrite\`
  reconnects it to hardware PWM function.
- The core \`analogWrite\` PWM preview supports **PA12..PA15 only**, and silently
  ignores unsupported pins; PWM4..PWM7 and adjustable PWM frequencies not yet done.
- \`digitalWrite()\` or \`pinMode()\` alone do **not** detach an active PWM mux
  in this experimental core. To drive a PWM pin constant, use \`analogWrite(pin,0)\`
  or \`analogWrite(pin,255)\`. Separate peripherals keep their existing pin mux.

## Installation

The ZIP installer targets the previously used Boards Manager package
\`%LOCALAPPDATA%\Arduino15\packages\nuc140\hardware\nuc140\0.1.2\`, falling back
only when exactly one other version exists. It modifies *only* the \`Arduino.h\`
prototype, adds \`wiring_pwm.c\`, and copies \`libraries/NUC140PWM\` with examples.
It does not replace the existing Arduino.h or other peripheral drivers.

## Examples

\`RGB_Colors\` cycles Red, Green, Blue, Yellow, Cyan, Magenta, White and Off.
\`RGB_Fade\` smoothly fades each channel, then all three channels together.
\`RGB_VR1_Dimmer\` combines tested ADC7/PA7 potentiometer with PWM0/1/2.

The RGB channel-to-color mapping follows Nu-LB board learning material;
users should report unexpected color ordering for the exact board variant.

## Sources

- https://www.nuvoton.com/resource-files/UM_Nu-LB-NUC140_EN_Rev2.0.pdf
- https://github.com/OpenNuvoton/NUC100BSP

## Reported hardware issue and diagnostic (2026-11-20)

The user reports: **"led hanya nyala merah tidak redup"** (only the red LED lights, with no fading). Therefore **do not label PWM hardware PASS**. The earlier v0.1.0 preview compiled on ARM but has not produced working PWM on the physical board.

A new `RGB_Diagnostic` example first tests GPIO ON/OFF on PA14 (RED), PA13 (GREEN) and PA12 (BLUE) independently, then applies `analogWrite(pin, 0/64/128/192/255)` to each channel and dumps CLK, GPIO alternate-function, PWM PPR/CSR/PCR/POE/CNR/CMR/SYNC registers at PWM=128. The GPIO stage distinguishes an RGB wiring/IO problem from PWM timer/mux setup. `Serial.print` register values alone do not validate physical PWM; observe the LED.

Compiled for ARM Cortex-M0; physical diagnostic results pending. Keep PR as draft until RGB Fade passes. **Do not merge** with earlier tested peripherals as a verified PWM release.

## v0.1.1 fix for PWM not fading (2026-11-20)

Physical-board diagnostic during `analogWrite(PA(14),128)`:

| Register | Readback |
|---|---|
| APBCLK | 0x00310001 (PWMA clocks enabled) |
| PWM_PPR | 0x00000101 |
| PWM_CSR | 0x00000000 (valid /2 divider) |
| PWM_SYNC | 0x0000FFFF |
| GPA_MFP, PWM_PCR, PWM_POE, PWM_CNR, PWM_CMR | 0x00000000 |

Diagnosis: **v0.1.0 aborted prematurely waiting for SYNCBUSY bit0
to clear**, before writing the counter, comparator, channel output
and pinmux. The patched v0.1.1 driver no longer uses that wait
to gate PWM setup, consistent with direct register programming used by
the Nuvoton StdDriver reference PWM_ConfigOutputChannel()/PWM_Start().

Hardware retest remains outstanding. Run RGB_Diagnostic from the v0.1.1
patch and send register readback plus visible RGB behavior. At
`analogWrite(PA(14),128)`, expected PWM2:
- `GPA_MFP` bit14 = 1;
- `PWM_PCR` CH2MOD bit19 and CH2EN bit16 = 1;
- `PWM_POE` bit2 = 1;
- `PWM_CNR` = 5529 (0x1599);
- `PWM_CMR` = 2775 (0x0AD7).

A continued PWM_SYNC=0xFFFF result alone is not evidence of failure;
verify the actual wave/dimming and CNR/CMR/POE readback. This is **not
hardware PASS** until user confirms. Existing UART, ADC, I2C, SPI,
LCD and RTC source files are unchanged.
