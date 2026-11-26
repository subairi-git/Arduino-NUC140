# SevenSegment4 Arduino library (Nu-LB-NUC140 V2.0)

Hardware validated 2026-11-26: original four-digit multiplex test displays correctly on the board.

Usage:

```cpp
#include <SevenSegment4.h>
void setup() {
  SevenSegment4.begin();
  SevenSegment4.display(1234);
}
void loop() {}
```

Methods: `begin()`, `display(uint16_t)` (0000–9999 with leading zeros, values modulo 10000), `clear()`, `end()`, `isRunning()`.

- Common anode: GPE0–GPE7 segment bus (active LOW); GPC4 rightmost through GPC7 leftmost, digit enable active HIGH.
- Glyph bytes for 0..9: `82 EE 07 46 6A 52 12 E6 02 62` (hex).
- Automatic scanning: one digit per 1ms SysTick, complete frame every 4ms (~250Hz/digit). No Timer0–Timer3 reserved; loop remains free for other work.
- The core's `SysTick_Handler` calls an optional weak `sevenSegment4Tick` hook when the library is linked. `millis()` continues to update.
- `display()` copies four glyph bytes atomically with PRIMASK restoration, safe to call while scanning.
- Hardware test of the *new library* and long-term concurrency testing with USB CDC/tone pending. Existing simple multiplex sketch was tested successfully.
- Resource conflict: this library owns GPE0..GPE7 and GPC4..GPC7. Other code must not reuse these pins during operation. Requires core's `wiring.c` optional hook; simply installing the library on an old core without the hook will not refresh the display.

Library code: `hardware/nuc140/libraries/SevenSegment4`. Example: `examples/SevenSegment4_Counter`.
