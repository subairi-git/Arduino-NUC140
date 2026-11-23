## Stage 2: 48MHz PLL without switching HCLK

Verified stage 1 on board by user: protected register access YES, 12MHz external crystal READY, CLKSTATUS 0x18 → 0x19, SysTick/UART alive.

Reference OpenNuvoton/NUC100BSP Library/StdDriver/src/clk.c: default HXT 48MHz PLL register = `0xC22E`.

`USB_CDC_PLL48_Preflight` enables 12MHz crystal and sets PLLCON to 0xC22E **only if PLL is not already in use**. It explicitly leaves CLKSEL0 unchanged, checks PLL stable, and logs Serial0 every second. This step does **not** enable USB device, create a USB descriptor, or enumerate a Windows COM port.

Expected output: `PASS: 48MHz PLL ready, CPU clock unchanged` and `alive millis=...`.

If failure occurs, retain Nu-Link upload access and restore a previous known-good sketch. Do not proceed to USB endpoint tests unless PLL ready.
