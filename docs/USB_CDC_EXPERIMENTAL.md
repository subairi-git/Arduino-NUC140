# USB CDC ACM initial integrated driver (experimental)

This branch adds a USB CDC ACM driver with USB enumeration, EP0 control (including descriptors, GET/SET_LINE_CODING, SET_CONTROL_LINE_STATE), EP2 bulk IN, EP3 bulk OUT, USB IRQ23, ring-buffered SerialUSB, and an echo example. **Hardware enumeration not yet confirmed!**

Test procedure: Upload the USB_CDC_Serial_Echo sketch through Nu-Link while keeping UART Serial Monitor open at 115200. Attach the separate USB Device J3 cable to Windows. Open Device Manager > Ports. Observe a second COM port or a new USB device. If unknown/Code 43, send error and UART log. If enumerated, open CDC COM and send text; it should be echoed.

`SerialUSB.begin(115200)` sets default line coding; host can change it. Unlike UART, baud is not a physical rate. `SerialUSB.connected()` indicates SET_CONFIGURATION, and boolean `SerialUSB` additionally requires DTR. `Serial` remains the existing UART0. USB endpoints and buffers are independent of timers.

**VID/PID notice:** The prototype uses Nuvoton's official USBD_VCOM sample VID/PID 0416:B002 only for local interoperability testing. Do not distribute or publish an official release using this ID without the rights to use it. Production needs an assigned VID/PID or authorized sub-license.

Source references: OpenNuvoton/NUC100BSP (Apache-2.0), especially StdDriver USBD_VCOM, usbd.c, NUC100Series.h.

Known limitations: no USB suspend power management, no asynchronous multi-port support, fixed 256 byte software RX/TX queues, control-transfer code and host interoperability need physical testing, no automatic USB-port-based sketch uploads. API is experimental; keep stable v0.1.5 untouched.

## Hardware test results — 2026-11-23

Board: Nu-LB-NUC140 V2.0, NUC140VE3CN. User reports that the USB CDC serial monitor displays repeated `Hello from NUC140 USB CDC` messages, and sending `TEST123` returns an echo `TEST123`. These results indicate basic host-to-device and device-to-host USB CDC transfers work. An earlier Windows Device Manager enumeration attempt showed Code 43 (`VID_0000&PID_0002`), but subsequent test produced working CDC message and echo; Windows device name and COM port number are not recorded. This report is user-observed rather than independently reproduced.

Pending validation before merging/release: cold plug/unplug under independent reliable power, USB bus reset and suspend/resume, repeated reconnect/re-enumeration, throughput/long transfers/ring-buffer overflow, stable coexistence with Timer/tone/SPI/I2C, and approved production VID/PID. Do not advertise release-ready yet.
