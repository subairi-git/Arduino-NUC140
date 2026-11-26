#include <stdint.h>
extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);
extern void (*__preinit_array_start[])(void), (*__preinit_array_end[])(void);
extern void (*__init_array_start[])(void), (*__init_array_end[])(void);
void Reset_Handler(void);
void Default_Handler(void);
void SysTick_Handler(void);
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EINT0_IRQHandler(void);
void EINT1_IRQHandler(void);
void GPAB_IRQHandler(void);
void GPCDEF_IRQHandler(void);
void TMR0_IRQHandler(void);
void TMR1_IRQHandler(void);
void TMR2_IRQHandler(void);
void TMR3_IRQHandler(void);
void USBD_IRQHandler(void);
void IRQ_Default(void) __attribute__((weak, alias("Default_Handler")));
// Reserved IRQs currently map to Default_Handler. No interrupts beyond SysTick supported.
__attribute__((section(".isr_vector"), used))
const uintptr_t vectors[] = {
 (uintptr_t)&_estack, (uintptr_t)Reset_Handler,
 (uintptr_t)NMI_Handler, (uintptr_t)HardFault_Handler,
 0,0,0,0,0,0,0, (uintptr_t)SVC_Handler,
 0,0,(uintptr_t)PendSV_Handler,(uintptr_t)SysTick_Handler,
 // NUC140 has 32 external IRQ slots; all disabled by default.
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)EINT0_IRQHandler,
 (uintptr_t)EINT1_IRQHandler,
 (uintptr_t)GPAB_IRQHandler,
 (uintptr_t)GPCDEF_IRQHandler,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)TMR0_IRQHandler,
 (uintptr_t)TMR1_IRQHandler,
 (uintptr_t)TMR2_IRQHandler,
 (uintptr_t)TMR3_IRQHandler,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)USBD_IRQHandler,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default,
 (uintptr_t)IRQ_Default
};
void Reset_Handler(void) {
 uint32_t *src=&_sidata;
 for (uint32_t *p=&_sdata;p<&_edata;) *p++=*src++;
 for (uint32_t *p=&_sbss;p<&_ebss;) *p++=0;
 for(void (**p)(void)=__preinit_array_start;p<__preinit_array_end;p++) (*p)();
 for(void (**p)(void)=__init_array_start;p<__init_array_end;p++) (*p)();
 (void)main();
 while(1){}
}
void Default_Handler(void){while(1){}}
