# Arduino analogWrite() hardware PWM for NUC140VE3CN

**Status: Preview.** NUC140 PWM0..PWM3 output, intended first hardware test
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
