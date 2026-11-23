# Tone / noTone — experimental

Arduino Uno-compatible API:

```cpp
tone(PB(10), 440);      // nonblocking, plays continuously
tone(PB(10), 523, 300); // 523 Hz for approximately 300 ms
noTone(PB(10));         // stop
```

**Wiring:** An **external passive buzzer** connected between a free GPIO pin (example PB10) and GND, within the pin's electrical limits. The on-board PB11 **active** buzzer only generates its own fixed pitch, even if driven by tone().

Implementation uses **Timer3 interrupts** to toggle GPIO; it offers one tone channel, frequency 31–10,000 Hz, approximate microsecond-based timing. tone() is nonblocking and uses half-period rounding (some pitch error). Duration counts interrupts and shuts down automatically. Calling tone() again replaces the previous note and silences its previous pin. noTone() stops only the currently playing pin.

**Resource conflict:** tone() will not take over a Timer3 already initialized with timerBegin(3,...). While tone owns Timer3, low-level Timer3 functions are restricted; an explicit timerBegin(3,...) replaces the tone and silences it. Timer0–2 are unaffected. Higher frequency notes increase interrupt CPU load.

Try Arduino IDE: File → Examples → Nuc140Examples → Tone_Passive_Melody.

Core functionality and board behavior should be confirmed with hardware test before the stable 0.1.6 release.
