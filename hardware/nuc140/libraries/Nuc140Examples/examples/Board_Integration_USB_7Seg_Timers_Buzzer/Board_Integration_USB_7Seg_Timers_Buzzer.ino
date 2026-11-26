#include <Arduino.h>
#include <SevenSegment4.h>

// Nu-LB-NUC140 V2.0 integrated test, NUC140VE3CN:
// - USB CDC on J3 (SerialUSB)
// - Common-anode four-digit 7-segment, GPE0..7 + GPC4..7 (SevenSegment4)
// - Timer0: 1Hz heartbeat, Timer1: 10Hz, Timer2: 20Hz
// - PB11 buzzer via tone() using Timer3
// - Nu-Link UART Serial at 115200 for independent diagnostic logs
// This sketch does not use any external components.
static volatile uint32_t timer0Ticks=0;
static volatile uint32_t timer1Ticks=0;
static volatile uint32_t timer2Ticks=0;
static bool timerOk=false;
static bool usbOk=false;
static bool beepEnabled=true;
static uint32_t lastReport=0;
static uint32_t lastBeep=0;

static void onTimer0() { ++timer0Ticks; }
static void onTimer1() { ++timer1Ticks; }
static void onTimer2() { ++timer2Ticks; }

static uint32_t atomicRead(volatile uint32_t &v) {
  uint32_t irqState;
  __asm__ volatile("mrs %0, primask\n cpsid i" : "=r"(irqState) :: "memory");
  uint32_t value=v;
  __asm__ volatile("msr primask, %0" :: "r"(irqState) : "memory");
  return value;
}
static void reportStatus() {
  const uint32_t t0=atomicRead(timer0Ticks);
  const uint32_t t1=atomicRead(timer1Ticks);
  const uint32_t t2=atomicRead(timer2Ticks);
  const uint32_t ms=millis();
  Serial.print("ms="); Serial.print(ms);
  Serial.print(" T0="); Serial.print(t0);
  Serial.print(" T1="); Serial.print(t1);
  Serial.print(" T2="); Serial.print(t2);
  Serial.print(" CDC="); Serial.print(SerialUSB.connected()?1:0);
  Serial.print(" DTR="); Serial.print(SerialUSB?1:0);
  Serial.print(" 7SEG="); Serial.print(SevenSegment4.isRunning()?1:0);
  Serial.print(" BEEP="); Serial.println(beepEnabled?1:0);
  if (SerialUSB && SerialUSB.availableForWrite()>100) {
    SerialUSB.print("NUC140 OK T0=");
    SerialUSB.print(t0);
    SerialUSB.print(" T1=");
    SerialUSB.print(t1);
    SerialUSB.print(" T2=");
    SerialUSB.println(t2);
  }
}
static void handleUsbInput() {
  // Type ? for status, B to enable beeps, S to silence, or any other char to echo.
  while(SerialUSB.available()>0) {
    const int c=SerialUSB.read();
    if(c<0)break;
    if(c=='B' || c=='b') {
      beepEnabled=true;
      SerialUSB.println("BEEP ON");
    } else if(c=='S' || c=='s') {
      beepEnabled=false;
      noTone(PB(11));
      SerialUSB.println("BEEP OFF");
    } else if(c=='?') {
      SerialUSB.println("CDC READY; 7SEG and Timers running");
    } else {
      SerialUSB.write((uint8_t)c);
    }
  }
}
void setup() {
  Serial.begin(115200);
  Serial.println("NUC140 BOARD INTEGRATION TEST");
  SevenSegment4.begin();
  SevenSegment4.display(1234);
  const bool t0=timerBegin(0,1000000UL,onTimer0);
  const bool t1=timerBegin(1,100000UL,onTimer1);
  const bool t2=timerBegin(2,50000UL,onTimer2);
  timerOk=t0 && t1 && t2;
  Serial.println(timerOk ? "TIMER0-2 STARTED" : "ERROR: one or more timers failed");
  usbOk=SerialUSB.begin(115200);
  Serial.println(usbOk ? "USB CDC STARTED" : "ERROR: USB CDC failed to start");
  lastReport=lastBeep=millis();
  Serial.println("Expect 7SEG 1234 then timer seconds; short beep every 5s.");
  Serial.println("USB CDC commands: ? status, B beep on, S beep off, others echo.");
}
void loop() {
  // Show real Timer0 tick count (not a millis() software counter).
  static uint32_t lastValue=0xFFFFFFFFUL;
  const uint32_t t0=atomicRead(timer0Ticks);
  if(t0!=lastValue) {
    lastValue=t0;
    SevenSegment4.display((uint16_t)(t0%10000UL));
  }
  handleUsbInput();
  const uint32_t now=millis();
  if((uint32_t)(now-lastBeep)>=5000UL) {
    lastBeep=now;
    if(beepEnabled)tone(PB(11),1200,120);  // Timer3; no conflict with T0–T2.
  }
  if((uint32_t)(now-lastReport)>=2000UL) {
    lastReport=now;
    reportStatus();
  }
}
