# Servo PWM3 50Hz test

**Target:** Nu-LB-NUC140 V2.0 (NUC140VE3CN), PA15 = PWM3.

This standalone sketch drives an external **standard 3-wire hobby servo** via true hardware PWM3 rather than `analogWrite()` (the existing analogWrite driver uses ~1kHz, unsuitable for RC servos).

## Wiring
- Orange/yellow signal -> PA15/PWM3
- Red VCC -> regulated external servo supply, usually 5V depending on model
- Brown/black GND -> external supply GND, **also connect to board GND**
- Do not draw motor current from a GPIO. Power off before rewiring; begin without mechanical load.
- NUC140 signal is 3.3V logic; verify your servo accepts this signal level. If not, use an appropriate signal level interface.

## How it works

Core assumes HCLK 22.1184MHz. PWM3 configured for /8 prescaler and /2 CSR divider (1.3824MHz tick), 27648 PWM ticks per 20ms frame. CMR3 determines high pulse width. Starts at 1500us (90°), then sweeps a *conservative* 90° -> 45° -> 90° -> 135° -> 90° sequence every 2 sec. Do not interpret positions as calibrated; modify bounds if the servo manufacturer specifies another range.

## Conflict caveat

**PWM2 PA14 and PWM3 PA15 share PWM prescaler CP23.** This example programs that prescaler for 50Hz; do not use `analogWrite(PA(14), ...)` or `analogWrite(PA(15), ...)` concurrently. PWM0/1 on PA12/PA13 are a separate pair; this code does not change CP01. If other code configures PWM3 independently, register ownership must be coordinated. This example is not yet hardware validated on a servo.

## Install and test

Clone/download `feature/servo-pwm-50hz` and open `PWM3_Servo_50Hz.ino` in Arduino IDE configured for NUC140 v0.1.6. Upload via Nu-Link; UART serial at 115200 prints angles. Example is not yet included in the published v0.1.6 Boards Manager package until a future tested release.
