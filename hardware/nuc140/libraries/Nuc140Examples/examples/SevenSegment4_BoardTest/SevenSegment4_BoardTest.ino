#include <Arduino.h>
/*
  Nu-LB-NUC140 V2.0 built-in four-digit seven-segment display TEST
  Board wiring from official Nuvoton user manual:
  PE0..PE7 = shared A B C D E F G DOT (segment bus)
  PC4..PC7 = select digit 1..4 (left to right).
  Transistor digit drivers and segment resistors are on the board.

  First experiment assumes segments active-LOW and digit selects active-HIGH.
  If the digits light inverted or nothing appears, try changing the flags below.
  This is a standalone Arduino sketch, compatible with USB CDC branch and v0.1.5.
*/
static const bool SEGMENT_ACTIVE_LOW = true;
static const bool DIGIT_ACTIVE_HIGH = true;
static const uint8_t SEG_MASK = 0xff;
static const uint8_t PATTERN_0_TO_9[10] = {
  0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F
}; // Bit0=A, bit1=B, ... bit6=G, bit7=DP
static uint8_t frame[4]={1,2,3,4};
static uint32_t lastDigit=0,lastFrame=0;
static uint8_t currentDigit=0;
static uint32_t phase=0;

static void selectAllOff(){
  for(uint8_t i=0;i<4;i++)digitalWrite(PC(4+i),DIGIT_ACTIVE_HIGH?LOW:HIGH);
}
static void putSegments(uint8_t bits){
  for(uint8_t i=0;i<8;i++){
    bool on=(bits & (1U<<i))!=0;
    digitalWrite(PE(i),SEGMENT_ACTIVE_LOW ? (on?LOW:HIGH) : (on?HIGH:LOW));
  }
}
static void refreshDigit(){
  // Turn OFF previous digit before shifting segment pattern, to avoid ghosting.
  selectAllOff();
  putSegments(frame[currentDigit]<=9 ? PATTERN_0_TO_9[frame[currentDigit]] : 0);
  digitalWrite(PC(4+currentDigit),DIGIT_ACTIVE_HIGH?HIGH:LOW);
  currentDigit=(currentDigit+1)&3U;
}
static void displayNumber(uint16_t n){
  frame[0]=(n/1000)%10;
  frame[1]=(n/100)%10;
  frame[2]=(n/10)%10;
  frame[3]=n%10;
}
void setup(){
  Serial.begin(115200);
  // No USB CDC dependency; this test also runs with UART alone.
  for(uint8_t i=0;i<8;i++)pinMode(PE(i),OUTPUT);
  for(uint8_t i=0;i<4;i++)pinMode(PC(4+i),OUTPUT);
  selectAllOff();
  putSegments(0);
  Serial.println("NUC140 built-in 4-digit 7segment test");
  Serial.println("PE0-PE7 segments; PC4-PC7 digit selects");
  Serial.println("Phase 0: 1234; phase 1: 8888; phase 2: 0000; phase 3: counter");
}
void loop(){
  uint32_t now=millis();
  // 2 ms per digit => 125 full display scans/s.
  if((uint32_t)(now-lastDigit)>=2UL){
    lastDigit=now;
    refreshDigit();
  }
  if((uint32_t)(now-lastFrame)>=3000UL){
    lastFrame=now;
    phase++;
    Serial.print("7SEG phase=");Serial.println(phase%4);
  }
  switch(phase%4){
    case 0: frame[0]=1;frame[1]=2;frame[2]=3;frame[3]=4;break;
    case 1: frame[0]=8;frame[1]=8;frame[2]=8;frame[3]=8;break;
    case 2: frame[0]=0;frame[1]=0;frame[2]=0;frame[3]=0;break;
    case 3: displayNumber((uint16_t)((now/100UL)%10000UL));break;
  }
}
