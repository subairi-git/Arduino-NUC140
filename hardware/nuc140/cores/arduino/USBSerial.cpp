// Experimental USB CDC ACM core implementation for NUC140VE3CN.
// Derived from register definitions and protocol flow in OpenNuvoton NUC100BSP
// USBD_VCOM (Apache-2.0); self-contained and independent of Nuvoton BSP headers.
// IMPORTANT: sample VID/PID 0416:B002 belongs to Nuvoton. TEST ONLY, not for release.
#include "Arduino.h"
#include "USBSerial.h"
#define M(a) (*(volatile uint32_t *)(a))
#define SYSREG 0x50000100UL
#define CLK_PWRCON 0x50000200UL
#define CLK_APBCLK 0x50000208UL
#define CLK_STATUS 0x5000020CUL
#define CLK_CLKSEL0 0x50000210UL
#define CLK_CLKDIV 0x50000218UL
#define CLK_PLLCON 0x50000220UL
#define USB 0x40060000UL
#define USB_RAM (USB+0x100UL)
#define USB_INTEN (USB+0x00UL)
#define USB_INTSTS (USB+0x04UL)
#define USB_FADDR (USB+0x08UL)
#define USB_ATTR (USB+0x10UL)
#define USB_FLDET (USB+0x14UL)
#define USB_STBUF (USB+0x18UL)
#define USB_SE0 (USB+0x90UL)
#define EP_BUF(i) (USB+0x20UL+16UL*(i))
#define EP_LEN(i) (USB+0x24UL+16UL*(i))
#define EP_CFG(i) (USB+0x28UL+16UL*(i))
#define EP_CTRL(i) (USB+0x2CUL+16UL*(i))
#define EP0_BUFFER 8UL
#define EP1_BUFFER 72UL
#define EP2_BUFFER 136UL
#define EP3_BUFFER 200UL
#define EP4_BUFFER 264UL
#define USB_BUS 1UL
#define USB_EVENT 2UL
#define USB_PLUG 4UL
#define USB_WAKE 8UL
#define USB_SETUP (1UL<<31)
static const uint8_t deviceDesc[]={
  18,1,0x10,0x01,2,0,0,64,
  0x16,0x04,0x02,0xB0,0,1,1,2,0,1
};
static const uint8_t configDesc[]={
  9,2,67,0,2,1,0,0x80,50,
  9,4,0,0,1,2,2,1,0,
  5,0x24,0,0x10,0x01,
  5,0x24,1,0,1,
  4,0x24,2,2,
  5,0x24,6,0,1,
  7,5,0x83,3,8,0,16,
  9,4,1,0,2,0x0A,0,0,0,
  7,5,0x81,2,64,0,0,
  7,5,0x02,2,64,0,0
};
static const uint8_t lang[]={4,3,9,4};
static const uint8_t maker[]={16,3,'N',0,'u',0,'v',0,'o',0,'t',0,'o',0,'n',0};
static const uint8_t product[]={32,3,
  'U',0,'S',0,'B',0,' ',0,'V',0,'i',0,'r',0,'t',0,
  'u',0,'a',0,'l',0,' ',0,'C',0,'O',0,'M',0};
static uint8_t setup[8],lineCoding[7]={0,0xC2,1,0,0,0,8};
static uint8_t ctrlAnswer[2];
static const uint8_t *ctrlRemain;
static uint16_t ctrlLeft;
static uint8_t setupAddress,addrPending,configuration,classLineOut,ctrlNeedZlp;
static uint8_t started;
static volatile uint8_t configured,dtr,txBusy;
static volatile uint16_t rxHead,rxTail,txHead,txTail;
static uint8_t rxData[256],txData[256];
static volatile uint32_t busResets, setups, bulkReceived,bulkSent;
static uint32_t irqSave(void) {
  uint32_t x;
  __asm__ volatile("mrs %0, primask\n cpsid i":"=r"(x)::"memory");
  return x;
}
static void irqRestore(uint32_t x) {
  __asm__ volatile("msr primask,%0"::"r"(x):"memory");
}
static void usbWrite(uint32_t off,const uint8_t *data,uint32_t size) {
  for(uint32_t i=0;i<size;i++)*(volatile uint8_t *)(USB_RAM+off+i)=data[i];
}
static void usbRead(uint8_t *data,uint32_t off,uint32_t size) {
  for(uint32_t i=0;i<size;i++)data[i]=*(volatile uint8_t *)(USB_RAM+off+i);
}
static void unlock(void) {
  M(SYSREG)=0x59;M(SYSREG)=0x16;M(SYSREG)=0x88;
}
static void stall(void) {
  M(EP_CTRL(0))|=2UL;M(EP_CTRL(1))|=2UL;
}
static void statusIn(void) {
  ctrlRemain=0;ctrlLeft=0;ctrlNeedZlp=0;
  M(EP_CFG(0))|=(1UL<<7);
  M(EP_LEN(0))=0;
}
static void dataIn(const uint8_t *p,uint16_t len,uint16_t requested) {
  if(len>requested)len=requested;
  uint16_t first=len>64?64:len;
  ctrlRemain=p+first;ctrlLeft=len-first;
  ctrlNeedZlp=(len && len<requested && (len%64U)==0U)?1U:0U;
  M(EP_CFG(0))|=(1UL<<7);
  usbWrite(EP0_BUFFER,p,first);
  M(EP_LEN(0))=first;
  M(EP_LEN(1))=64;
}
static void resetUSB(void) {
  ++busResets;
  configured=0;configuration=0;dtr=0;
  addrPending=0;ctrlRemain=0;ctrlLeft=0;ctrlNeedZlp=0;
  classLineOut=0;txBusy=0;
  rxHead=rxTail=txHead=txTail=0;
  M(USB_FADDR)=0;
  for(unsigned i=0;i<5;i++)M(EP_CFG(i))&=~(1UL<<7);
  M(EP_LEN(3))=64;
}
static void configureEndpoints(void) {
  M(USB_STBUF)=0; // setup packet at offset 0
  M(EP_BUF(0))=EP0_BUFFER;
  M(EP_CFG(0))=(1UL<<9)|(2UL<<5); // control IN endpoint 0
  M(EP_BUF(1))=EP1_BUFFER;
  M(EP_CFG(1))=(1UL<<9)|(1UL<<5); // control OUT endpoint 0
  M(EP_BUF(2))=EP2_BUFFER;
  M(EP_CFG(2))=(2UL<<5)|1UL; // bulk IN address 1
  M(EP_BUF(3))=EP3_BUFFER;
  M(EP_CFG(3))=(1UL<<5)|2UL; // bulk OUT address 2
  M(EP_BUF(4))=EP4_BUFFER;
  M(EP_CFG(4))=(2UL<<5)|3UL; // interrupt IN address 3
  M(EP_LEN(3))=64;
}
static void requestSetup(void) {
  usbRead(setup,0,8);++setups;
  M(EP_CTRL(0))|=1UL;
  M(EP_CTRL(1))|=1UL;
  ctrlRemain=0;ctrlLeft=0;ctrlNeedZlp=0;
  classLineOut=0;
  uint8_t bm=setup[0],request=setup[1];
  uint16_t value=(uint16_t)setup[2]|((uint16_t)setup[3]<<8);
  uint16_t length=(uint16_t)setup[6]|((uint16_t)setup[7]<<8);
  if((bm&0x60U)==0) {
    if(request==6 && (bm&0x80U)) {
      const uint8_t *p=0;uint16_t n=0;
      switch(value>>8) {
        case 1: p=deviceDesc;n=sizeof(deviceDesc);break;
        case 2: p=configDesc;n=sizeof(configDesc);break;
        case 3:
          switch(value&255) {
            case 0:p=lang;n=sizeof(lang);break;
            case 1:p=maker;n=sizeof(maker);break;
            case 2:p=product;n=sizeof(product);break;
          }
          break;
      }
      if(p){dataIn(p,n,length);return;}
    } else if(request==5 && bm==0) {
      setupAddress=(uint8_t)(value&127);addrPending=1;statusIn();return;
    } else if(request==9 && bm==0 && value<=1) {
      configuration=(uint8_t)value;configured=(uint8_t)value;
      statusIn();return;
    } else if(request==8 && bm==0x80) {
      ctrlAnswer[0]=configuration;dataIn(ctrlAnswer,1,length);return;
    } else if(request==0 && (bm&0x80)) {
      ctrlAnswer[0]=0;ctrlAnswer[1]=0;dataIn(ctrlAnswer,2,length);return;
    } else if(request==10 && bm==0x81) {
      ctrlAnswer[0]=0;dataIn(ctrlAnswer,1,length);return;
    } else if(request==11 && bm==0x01 && value==0) {
      statusIn();return;
    } else if((request==1 || request==3) && !(bm&0x80)) {
      statusIn();return;
    }
  } else if((bm&0x60U)==0x20U && (setup[4]==0)) {
    if(request==0x21 && (bm&0x80U)) {
      dataIn(lineCoding,7,length);return;
    } else if(request==0x20 && !(bm&0x80U) && length==7) {
      classLineOut=1;
      M(EP_LEN(1))=64;
      return;
    } else if(request==0x22 && !(bm&0x80U)) {
      dtr=(value&1U)?1U:0U;
      statusIn();return;
    } else if(request==0x23 && !(bm&0x80U)) {
      statusIn();return;
    }
  }
  stall();
}
static void controlAck(void) {
  if(ctrlLeft) {
    uint16_t n=ctrlLeft>64?64:ctrlLeft;
    usbWrite(EP0_BUFFER,ctrlRemain,n);
    ctrlRemain+=n;ctrlLeft-=n;
    M(EP_LEN(0))=n;
  } else if(ctrlNeedZlp) {
    ctrlNeedZlp=0;
    M(EP_LEN(0))=0;
  } else if(addrPending) {
    M(USB_FADDR)=setupAddress;addrPending=0;
  }
}
static void txKick(void) {
  if(!configured || txBusy || txTail==txHead)return;
  uint8_t packet[64];unsigned n=0;
  while(n<sizeof(packet) && txTail!=txHead) {
    packet[n++]=txData[txTail];
    txTail=(uint8_t)(txTail+1);
  }
  usbWrite(EP2_BUFFER,packet,n);
  txBusy=1;M(EP_LEN(2))=n;
}
static void bulkOut(void) {
  uint32_t n=M(EP_LEN(3))&0x1FFUL;
  if(n>64)n=64;
  // Do not overwrite unread data. Leave OUT NAKed until main drains RX.
  uint16_t available=(uint8_t)(rxHead-rxTail);
  if(n && (255U-available)<n) {
    // Drop full packet rather than deadlocking USB transaction.
    M(EP_LEN(3))=64;
    return;
  }
  for(unsigned i=0;i<n;i++) {
    rxData[rxHead]=(uint8_t)M(USB_RAM+EP3_BUFFER+i); // byte access mode
    rxHead=(uint8_t)(rxHead+1);
  }
  if(n)bulkReceived+=n;
  M(EP_LEN(3))=64;
}
extern "C" void USBD_IRQHandler(void) {
  uint32_t st=M(USB_INTSTS);
  if(st&USB_PLUG) {
    M(USB_INTSTS)=USB_PLUG;
    if(M(USB_FLDET)&1)M(USB_ATTR)|=(1UL<<7)|(1UL<<4);
    else {M(USB_ATTR)&=~(1UL<<7);configured=0;dtr=0;}
  }
  if(st&USB_BUS) {
    M(USB_INTSTS)=USB_BUS;
    uint32_t a=M(USB_ATTR);
    if(a&1U){M(USB_ATTR)|=(1UL<<7)|(1UL<<4);resetUSB();}
    if(a&2U)M(USB_ATTR)&=~(1UL<<4);
    if(a&4U)M(USB_ATTR)|=(1UL<<7)|(1UL<<4);
  }
  if(st&USB_WAKE)M(USB_INTSTS)=USB_WAKE;
  if(st&USB_EVENT) {
    if(st&USB_SETUP) {
      M(USB_INTSTS)=USB_SETUP;
      requestSetup();
    }
    if(st&(1UL<<16)) {
      M(USB_INTSTS)=(1UL<<16);
      controlAck();
    }
    if(st&(1UL<<17)) {
      M(USB_INTSTS)=(1UL<<17);
      if(classLineOut) {
        usbRead(lineCoding,EP1_BUFFER,7);
        classLineOut=0;statusIn();
      }
      M(EP_LEN(1))=64;
    }
    if(st&(1UL<<18)) {
      M(USB_INTSTS)=(1UL<<18);
      txBusy=0;++bulkSent;
    }
    if(st&(1UL<<19)) {
      M(USB_INTSTS)=(1UL<<19);
      bulkOut();
    }
    if(st&(1UL<<20))M(USB_INTSTS)=(1UL<<20);
  }
}
USBSerial SerialUSB;
USBSerial::USBSerial(){}
bool USBSerial::begin(uint32_t baud) {
  (void)baud; // CDC line coding negotiated by host; not UART baud clock.
  if(started)return true;
  if((M(CLK_CLKSEL0)&7UL)==2UL)return false; // do not reprogram PLL used by HCLK
  uint32_t saved=irqSave();unlock();
  bool ok=(M(SYSREG)&1UL)!=0UL;
  if(ok)M(CLK_PWRCON)|=1UL;
  M(SYSREG)=0;irqRestore(saved);if(!ok)return false;
  uint32_t start=millis();
  while(!(M(CLK_STATUS)&1UL) && millis()-start<1500UL){}
  if(!(M(CLK_STATUS)&1UL))return false;
  saved=irqSave();unlock();
  ok=(M(SYSREG)&1UL)!=0UL;
  if(ok) {
    M(CLK_PLLCON)=0xC22EUL;
    M(CLK_CLKDIV)&=~(15UL<<4);
    M(CLK_APBCLK)|=(1UL<<27);
  }
  M(SYSREG)=0;irqRestore(saved);if(!ok)return false;
  start=millis();
  while(!(M(CLK_STATUS)&4UL) && millis()-start<1500UL){}
  if(!(M(CLK_STATUS)&4UL))return false;
  configured=0;dtr=0;txBusy=0;
  rxHead=rxTail=txHead=txTail=0;
  M(USB_INTEN)=0;
  M(USB_ATTR)=0x7D0UL; // USB PHY/clock and byte-access mode, from Nuvoton BSP
  M(USB_SE0)=1UL; // software disconnect before advertising descriptors
  configureEndpoints();
  delay(110);
  M(USB_INTSTS)=USB_BUS|USB_EVENT|USB_PLUG|USB_WAKE;
  M(USB_INTEN)=USB_BUS|USB_EVENT|USB_PLUG|USB_WAKE;
  // NUC100 USBD_IRQn = 23, NVIC bit 23
  M(0xE000E280UL)=1UL<<23;
  M(0xE000E100UL)=1UL<<23;
  started=1;
  M(USB_SE0)=0;
  if(M(USB_FLDET)&1UL)M(USB_ATTR)|=(1UL<<7)|(1UL<<4);
  return true;
}
void USBSerial::end() {
  if(!started)return;
  M(0xE000E180UL)=1UL<<23;
  M(USB_INTEN)=0;
  M(USB_SE0)=1UL;
  M(USB_ATTR)&=~((1UL<<7)|(1UL<<8));
  configured=dtr=started=0;
}
USBSerial::operator bool() const {return started && configured && dtr;}
bool USBSerial::connected() const {return started && configured;}
void USBSerial::poll() {
  if(!started)return;
  uint32_t x=irqSave();txKick();irqRestore(x);
}
int USBSerial::available() {
  uint32_t x=irqSave();int n=(uint8_t)(rxHead-rxTail);irqRestore(x);return n;
}
int USBSerial::availableForWrite() {
  uint32_t x=irqSave();int n=255-(uint8_t)(txHead-txTail);irqRestore(x);return n;
}
int USBSerial::peek(){
  uint32_t x=irqSave();int v=rxHead==rxTail?-1:rxData[rxTail];irqRestore(x);return v;
}
int USBSerial::read(){
  uint32_t x=irqSave();int v=-1;
  if(rxTail!=rxHead){v=rxData[rxTail];rxTail=(uint8_t)(rxTail+1);}
  irqRestore(x);return v;
}
size_t USBSerial::write(uint8_t b){
  if(!started || !configured)return 0;
  uint32_t x=irqSave();
  uint8_t next=(uint8_t)(txHead+1);
  if(next==txTail){irqRestore(x);return 0;}
  txData[txHead]=b;txHead=next;
  txKick();irqRestore(x);return 1;
}
size_t USBSerial::write(const uint8_t *p,size_t n){
  if(!p)return 0;
  size_t sent=0;while(sent<n && write(p[sent]))sent++;
  return sent;
}
size_t USBSerial::write(const char *s){
  if(!s)return 0;
  size_t n=0;while(*s && write((uint8_t)*s++))n++;
  return n;
}
void USBSerial::flush(){
  uint32_t start=millis();
  while(started && configured && (txHead!=txTail || txBusy) &&
        millis()-start<1500UL)poll();
}
size_t USBSerial::print(const char *s){return write(s);}
size_t USBSerial::print(char s){return write((uint8_t)s);}
size_t USBSerial::printNumber(unsigned long v,int base){
  if(base==0)return write((uint8_t)v);
  if(base<2 || base>36)base=10;
  char b[33];unsigned p=sizeof(b);b[--p]=0;
  do {unsigned d=v%base;b[--p]=d<10?'0'+d:'A'+d-10;v/=base;}while(v);
  return write(&b[p]);
}
size_t USBSerial::print(unsigned long n,int b){return printNumber(n,b);}
size_t USBSerial::print(unsigned int n,int b){return printNumber(n,b);}
size_t USBSerial::print(long n,int b){
  if(n<0 && b==10){size_t k=write((uint8_t)'-');return k+printNumber((unsigned long)(-(n+1))+1UL,b);}
  return printNumber((unsigned long)n,b);
}
size_t USBSerial::print(int n,int b){return print((long)n,b);}
size_t USBSerial::println(){return write("\r\n");}
size_t USBSerial::println(const char *s){return print(s)+println();}
size_t USBSerial::println(int v,int b){return print(v,b)+println();}
size_t USBSerial::println(unsigned int v,int b){return print(v,b)+println();}
size_t USBSerial::println(long v,int b){return print(v,b)+println();}
size_t USBSerial::println(unsigned long v,int b){return print(v,b)+println();}
