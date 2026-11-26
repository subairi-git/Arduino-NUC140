#pragma once
#include <stdint.h>

// Built-in display on Nu-LB-NUC140 V2.0, common anode.
// GPE0..GPE7 = segments (active LOW), GPC4..GPC7 = digit selects (active HIGH).
// Independent of hardware timers: a 1ms SysTick hook performs multiplexing.
class SevenSegment4Class {
public:
  void begin();
  void end();
  void display(uint16_t number);  // 0000 through 9999, leading zeroes
  void clear();
  bool isRunning() const;
};
extern SevenSegment4Class SevenSegment4;
