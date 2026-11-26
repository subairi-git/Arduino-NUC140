# Nu-LB-NUC140 V2.0 4-digit 7-segment test

Source: official Nuvoton Nu-LB-NUC140 User Manual Rev 2.00 p. 9 (7-SEGMENT function GPE0–GPE7 and GPC4–GPC7).

Sketch `SevenSegment4_BoardTest.ino` scans 4 digits using 2ms/digit multiplex, shows 1234, 8888, 0000, then a counter; logs phase on UART Serial 115200. Default assumes PE segment active low and PC digit select active high; these polarity hypotheses are **not hardware verified** and may need inversion according to the physical board. DP is off by default. A–G mapping is assumed PE0=A through PE6=G and PE7=DP; verify on board and adjust if glyphs are malformed. Board manual confirms pin groups, not individual segment ordering in source text.

USB CDC is independent; no SerialUSB calls in this sketch. Keep Nu-Link programming connected; no need to disconnect J3 if it is power source. No external wiring is required.
