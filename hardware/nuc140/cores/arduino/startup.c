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
void IRQ_Default(void) __attribute__((weak, alias("Default_Handler")));
// NUC140 IRQ4/IRQ5 GPIO and IRQ8-IRQ11 hardware timers.
void GPAB_IRQHandler(void);
void GPCDEF_IRQHandler(void);
void TMR0_IRQHandler(void);
void TMR1_IRQHandler(void);
void TMR2_IRQHandler(void);
void TMR3_IRQHandler(void);
// Unimplemented external IRQs continue to use Default_Handler.
__attribute__((section(".isr_vector"), used))
const uintptr_t vectors[] = {
 (uintptr_t)&_estack, (uintptr_t)Reset_Handler,
 (uintptr_t)NMI_Handler, (uintptr_t)HardFault_Handler,
 0,0,0,0,0,0,0, (uintptr_t)SVC_Handler,
 0,0,(uintptr_t)PendSV_Handler,(uintptr_t)SysTick_Handler,
 // NUC140 external IRQ numbers 0..31, corresponding to vector slots 16..47.
 (uintptr_t)IRQ_Default, // IRQ0
 (uintptr_t)IRQ_Default, // IRQ1
 (uintptr_t)IRQ_Default, // IRQ2
 (uintptr_t)IRQ_Default, // IRQ3
 (uintptr_t)GPAB_IRQHandler, // IRQ4
 (uintptr_t)GPCDEF_IRQHandler, // IRQ5
 (uintptr_t)IRQ_Default, // IRQ6
 (uintptr_t)IRQ_Default, // IRQ7
 (uintptr_t)TMR0_IRQHandler, // IRQ8
 (uintptr_t)TMR1_IRQHandler, // IRQ9
 (uintptr_t)TMR2_IRQHandler, // IRQ10
 (uintptr_t)TMR3_IRQHandler, // IRQ11
 (uintptr_t)IRQ_Default, // IRQ12
 (uintptr_t)IRQ_Default, // IRQ13
 (uintptr_t)IRQ_Default, // IRQ14
 (uintptr_t)IRQ_Default, // IRQ15
 (uintptr_t)IRQ_Default, // IRQ16
 (uintptr_t)IRQ_Default, // IRQ17
 (uintptr_t)IRQ_Default, // IRQ18
 (uintptr_t)IRQ_Default, // IRQ19
 (uintptr_t)IRQ_Default, // IRQ20
 (uintptr_t)IRQ_Default, // IRQ21
 (uintptr_t)IRQ_Default, // IRQ22
 (uintptr_t)IRQ_Default, // IRQ23
 (uintptr_t)IRQ_Default, // IRQ24
 (uintptr_t)IRQ_Default, // IRQ25
 (uintptr_t)IRQ_Default, // IRQ26
 (uintptr_t)IRQ_Default, // IRQ27
 (uintptr_t)IRQ_Default, // IRQ28
 (uintptr_t)IRQ_Default, // IRQ29
 (uintptr_t)IRQ_Default, // IRQ30
 (uintptr_t)IRQ_Default // IRQ31
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
