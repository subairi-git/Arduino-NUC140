# USB CDC (Virtual COM Port) — hardware preflight

**Status:** hardware preflight only; there is NO functional `SerialUSB` implementation yet. Keep v0.1.5 as production baseline.

**Board:** Nu-LB-NUC140 V2.0 / NUC140VE3CN.

The board has a separate USB Device J3 mini-B connector; Nu-Link programming USB is a different connector. This family has a USBD full-speed controller. Nuvoton's NUC100BSP sample under `SampleCode/StdDriver/USBD_VCOM` provides USB ACM descriptors, endpoint allocation and USB interrupt handling under an Apache-2.0 license.

**USB clock constraint:** full-speed USB peripheral clock needs 48 MHz; the existing Arduino core assumes HIRC 22.1184 MHz for CPU, timers, SysTick and UART. Reconfiguring the system HCLK to 48 MHz would break existing timing. Plan: use board external 12MHz crystal and configure 48MHz PLL for USB without changing HCLK, and verify each affected clock register against the NUC140 reference manual before implementing.

## Run the hardware probe
Open `File > Examples > Nuc140Examples > USB_CDC_Clock_Preflight` after installing the example source on a development core. With the existing UART Serial Monitor at 115200, expect:

- `Protected registers unlocked: YES`
- `12MHz external crystal: READY`
- Periodic `alive millis=...` messages while LED PC12 toggles.

The probe deliberately does not enable USBD or announce any USB device, so Windows is *not* expected to show a new COM port.

## Implementation checklist
1. Establish stable USB 48MHz peripheral clock while retaining existing 22.1184MHz HCLK.
2. Implement and test USBD interrupt 23 vector, attach/detach and control endpoint.
3. Add CDC ACM device/configuration/string descriptors, SET/GET_LINE_CODING, SET_CONTROL_LINE_STATE and endpoint 1/2 bulk data handling.
4. Expose `SerialUSB.begin()`, `available()`, `read()`, `write()`, `print()`, `println()`, independent from UART0 `Serial`.
5. Test enumeration, Windows Device Manager COM port, bidirectional echo, disconnect/reconnect, USB reset/suspend, and coexistence with timers.

**USB reference:** https://github.com/OpenNuvoton/NUC100BSP/tree/master/SampleCode/StdDriver/USBD_VCOM

This plan and probe are not a completed USB CDC implementation.
