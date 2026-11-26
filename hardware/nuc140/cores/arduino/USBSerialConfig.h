#pragma once
// USB VID/PID are not assigned by this experimental community core.
// To enable USB CDC, configure BOTH identifiers that you are authorized
// to use. Do NOT use the Nuvoton sample 0416:B002 in distributed firmware.
//
// For example, set compiler flags in your local platform.local.txt:
//   compiler.usb.flags=-DNUC140_USB_VID=<your_authorized_vid> -DNUC140_USB_PID=<your_authorized_pid>
// and add {compiler.usb.flags} to compiler.common.flags in your local
// platform.local.txt override. Or edit the two zero defaults below
// in your private board installation ONLY (not in the public repository).
#ifndef NUC140_USB_VID
#define NUC140_USB_VID 0x0000U
#endif
#ifndef NUC140_USB_PID
#define NUC140_USB_PID 0x0000U
#endif
#if (NUC140_USB_VID > 0xFFFFU) || (NUC140_USB_PID > 0xFFFFU)
#error "USB VID/PID must be 16-bit numbers"
#endif
