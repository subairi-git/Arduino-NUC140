#include <Arduino.h>
volatile uint32_t c0=0,c1=0,c2=0,c3=0;
void t0(){++c0;} void t1(){++c1;} void t2(){++c2;} void t3(){++c3;}
void setup(){
 Serial.begin(115200);
 bool ok=timerBegin(0,1000000UL,t0);
 ok=timerBegin(1,500000UL,t1)&&ok;
 ok=timerBegin(2,250000UL,t2)&&ok;
 ok=timerBegin(3,100000UL,t3)&&ok;
 Serial.println(ok?"TIMER 0-3 OK":"TIMER INIT FAIL");
}
void loop(){
 static uint32_t last=0;
 if(millis()-last>=1000){
   last=millis();
   noInterrupts();uint32_t a=c0,b=c1,c=c2,d=c3;interrupts();
   Serial.print("T0=");Serial.print(a);Serial.print(" T1=");Serial.print(b);
   Serial.print(" T2=");Serial.print(c);Serial.print(" T3=");Serial.println(d);
 }
}
