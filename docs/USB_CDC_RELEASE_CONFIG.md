# USB CDC for v0.1.6

The public release DOES NOT include an assigned USB vendor/product identifier. USB CDC is **disabled by default**: `SerialUSB.begin()` returns false when `NUC140_USB_VID` or `NUC140_USB_PID` is 0.

To enable USB CDC, provide two identifiers that you own or are authorized to use. In a private local copy of the installed core, edit `cores/arduino/USBSerialConfig.h` to define your assigned 16-bit VID and PID. Do not commit or publish identifiers obtained without authorization. The published device descriptor reads these macros, and a default 0/0 pair does not enable the USB pull-up.

Hardware validation with development-only settings has confirmed bidirectional CDC on J3, but the default public release intentionally does not enumerate until configured.

Keep Nu-Link for upload/programming. The USB CDC connector J3 carries data and may supply board power, depending on hardware setup.

If you do not have an authorized VID/PID, use UART `Serial` for now; USB CDC source remains available for research/development without shipping an impersonated USB identity.
