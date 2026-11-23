## Stage 3 — USB Device J3 VBUS detection

This diagnostic enables the APBCLK USBD module clock (bit 27) and the USB clock divider /1 after configuring the HXT 12MHz to PLL 48MHz. It **does not enable the USB D+ pull-up** and **cannot enumerate a CDC COM port**.

Keep Nu-Link programming interface connected for uploading and Serial UART log, then plug/unplug the separate USB Device J3 cable. Observe FLDET register bit 0 (`ATTACHED` / `DETACHED`). If the bit does not change, check the USB Device port and cable. The core's SysTick and UART remain on unchanged HCLK.

Hardware register references: OpenNuvoton NUC100BSP `NUC100Series.h` (APBCLK USBD enable bit 27; USBD base 0x40060000, FLDET +0x14; CLKDIV USB shift 4). The test intentionally does not initialize CDC class/USB endpoints.
