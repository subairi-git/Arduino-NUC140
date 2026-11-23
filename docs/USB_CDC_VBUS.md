## Stage 3 — USB Device J3 VBUS detection

This diagnostic enables the APBCLK USBD module clock (bit 27) and the USB clock divider /1 after configuring the HXT 12MHz to PLL 48MHz. It **does not enable the USB D+ pull-up** and **cannot enumerate a CDC COM port**.

Keep Nu-Link programming interface connected for uploading and Serial UART log, then plug/unplug the separate USB Device J3 cable. Observe FLDET register bit 0 (`ATTACHED` / `DETACHED`). If the bit does not change, check the USB Device port and cable. The core's SysTick and UART remain on unchanged HCLK.

Hardware register references: OpenNuvoton NUC100BSP `NUC100Series.h` (APBCLK USBD enable bit 27; USBD base 0x40060000, FLDET +0x14; CLKDIV USB shift 4). The test intentionally does not initialize CDC class/USB endpoints.

## Physical test: USB Device J3 attached (23 November 2026)

Observed on user board:

- `CLKSTATUS=0x1D`: HXT and PLL stable.
- `APBCLK=0x8010001`: USBD module clock enabled (bit 27).
- `CLKDIV=0`, `CLKSEL0=0x3F`: USB divider /1, CPU clock selection preserved.
- `FLDET=1`, `ATTR=0x40`, `INTSTS=0x4`: USB cable attached and plug detection event pending, but USB device NOT enabled and no CDC enumeration performed.
- UART Serial and millis() continued normally.

**Unverified:** unplug transition (FLDET 1→0), USB enumeration, CDC descriptors, Windows COM port, bidirectional transfers. Future work must add endpoint 0 control handling and CDC ACM class before claiming SerialUSB support.
