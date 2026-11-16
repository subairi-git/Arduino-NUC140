#include "Arduino.h"
extern void setup(void);
extern void loop(void);
extern "C" int main(void) {
  init();
  setup();
  for (;;) { loop(); yield(); }
}
