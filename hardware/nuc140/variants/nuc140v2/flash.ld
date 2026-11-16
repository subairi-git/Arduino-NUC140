/* Only NUC140VE3CN: 128K APROM and 16K SRAM. Confirm chip marking. */
ENTRY(Reset_Handler)
MEMORY { FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 128K
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 16K }
_estack = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
  .vectors : { . = ALIGN(4); KEEP(*(.isr_vector)) } > FLASH
  .text : { . = ALIGN(4); *(.text*) *(.rodata*) KEEP(*(.init)) KEEP(*(.fini)) . = ALIGN(4); } > FLASH
  .preinit_array : { . = ALIGN(4); __preinit_array_start = .; KEEP(*(.preinit_array*)) __preinit_array_end = .; } > FLASH
  .init_array : { . = ALIGN(4); __init_array_start = .; KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array*)) __init_array_end = .; } > FLASH
  .fini_array : { . = ALIGN(4); __fini_array_start = .; KEEP(*(.fini_array*)) __fini_array_end = .; } > FLASH
  .ARM.exidx : { *(.ARM.exidx*) } > FLASH
  . = ALIGN(4);
  _sidata = LOADADDR(.data);
  .data : { . = ALIGN(4); _sdata = .; *(.data*) . = ALIGN(4); _edata = .; } > RAM AT> FLASH
  .bss (NOLOAD) : { . = ALIGN(4); _sbss = .; *(.bss*) *(COMMON) . = ALIGN(4); _ebss = .; } > RAM
  . = ALIGN(8);
  _end = .;
  PROVIDE(end = .);
  ASSERT(_end < _estack - 1024, "Insufficient RAM for stack")
}
