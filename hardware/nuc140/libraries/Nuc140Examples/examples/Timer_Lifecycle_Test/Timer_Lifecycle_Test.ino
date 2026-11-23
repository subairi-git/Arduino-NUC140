#include <Arduino.h>
volatile uint32_t ticks=0;
uint8_t phase=0;
uint32_t phaseAt=0,lastPrint=0;
void onTick(){++ticks;}
void setup(){
 Serial.begin(115200);
 Serial.println("NUC140 Timer Lifecycle Test");
 Serial.println("START: Timer0 2Hz");
 if(!timerBegin(0,500000UL,onTick)) Serial.println("TIMER0 INIT FAILED");
 phaseAt=millis();
}
void loop(){
 uint32_t now=millis();
 if(now-lastPrint>=1000){
   lastPrint=now;
   Serial.print("phase=");Serial.print(phase);
   Serial.print(" millis=");Serial.print(now);
   Serial.print(" ticks=");Serial.println(ticks);
 }
 if(now-phaseAt>=4000 && phase<4){
   phaseAt=now;++phase;
   switch(phase){
     case 1: timerStop(0); Serial.println("STOP: count should hold for 4 seconds");break;
     case 2: timerStart(0);Serial.println("RESTART: 2Hz resumes");break;
     case 3: timerSetPeriod(0,250000UL);Serial.println("PERIOD: 4Hz");break;
     case 4: timerEnd(0);Serial.println("END: count should hold indefinitely");break;
   }
 }
}
