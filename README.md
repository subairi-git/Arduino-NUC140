# Arduino NUC140 V2.0 – Experimental v0.1.2

Unofficial Arduino core for **Nu-LB-NUC140 V2.0 (2014), NUC140VE3CN** (Cortex-M0).
**Experimental / not yet tested on real hardware**. It supports basic GPIO and timing; standard Serial, SPI, I2C, PWM and ADC are not implemented. Verify the exact MCU marking before use.

## Installation via Arduino IDE Boards Manager

1. Publish this repo under `https://github.com/subairi-git/Arduino-NUC140` with default branch `main`.
2. Enable GitHub Actions **Settings → Actions → General → Workflow permissions: Read and write permissions**.
3. Push release tag `v0.1.2` **only after** committing all files from this version; workflow builds a single-root `.tar.gz`, publishes the GitHub Release and updates the index on `main`.
4. Arduino IDE → File → Preferences → Additional Boards Manager URLs:
   `https://raw.githubusercontent.com/subairi-git/Arduino-NUC140/main/package_nuc140_index.json`
5. Boards Manager → search `NUC140` → choose **0.1.2**. Select board **Nu-LB-NUC140 V2.0 (NUC140VE3CN)**.

## Windows direct Upload via Nu-Link SWD (new in v0.1.2)

You need **Nu-Link (or Nu-Link-Me)** connected via its programming USB connector and Nuvoton's official **Nu-Link Command Tool** installed. This is **not a COM port upload**. The Arduino core does **not** bundle NuLink.exe or its driver.

1. Close any ICP, ISP, debugger or other application using Nu-Link.
2. In Windows CMD, first check that the official command-line tool can detect the chip (use the path that matches your installation):

   ```cmd
   "C:\Program Files\Nuvoton Tools\NuLink Command Tool\NuLink.exe" -l
   "C:\Program Files\Nuvoton Tools\NuLink Command Tool\NuLink.exe" -p
   ```

   On some PCs it lives under `C:\Program Files (x86)\Nuvoton Tools\NuLink Command Tool\NuLink.exe`.
   The `-p` output **must** contain `NUC140VE3CN`; stop if it does not.
3. In Arduino IDE choose **File → Examples → Nuc140Examples → BlinkOnboard** (uses PC12/active LOW), then click **Verify** first.
4. After Verify succeeds, click **Upload (→)**. The core's `tools/nulink-upload.ps1` will locate the official NuLink.exe and execute the APROM-only sequence after verifying the firmware's ARM vector table and chip identity:
   `NuLink -e APROM` → `NuLink -w APROM <firmware.bin>` → `NuLink -v APROM <firmware.bin>` → `NuLink -reset` → `NuLink -disconnect`.
5. Monitor the IDE output for `[NUC140] SUCCESS`. If it fails, enable verbose output in File → Preferences and copy the full log.

If NuLink.exe is installed somewhere else, set a Windows user environment variable named **`NUC140_NULINK_EXE`** to the complete file path (e.g. `D:\Tools\NuLink.exe`) and **restart Arduino IDE**. The script searches common `Program Files` locations and PATH automatically.

**Important warnings**

- Upload **erases the entire APROM application region**. Back up your original application with the official tool first if you need to preserve it.
- This script does **not** use `-e all`; it never intentionally changes LDROM, DataFlash or configuration bits. It does not modify boot source settings. If your chip is configured to boot from LDROM, blinking may not start until the boot configuration is correctly understood.
- The upload will refuse to proceed unless the chip identifies as **NUC140VE3CN** and the binary has a plausible Cortex-M vector table and is at most 128 KB.
- If Nu-Link Command Tool reports a different chip ID format, provide the **full output of `NuLink.exe -p`** for a targeted compatibility update; do not bypass the check blindly.
- Windows/PowerShell 5.1 uploader is **not yet exercised on real NUC140 hardware**. Report the first error encountered.

## Existing experimental core behavior

- GPIO numbering: `PA(0..15)=0..15`, `PB(0..15)=16..31`, `PC(0..15)=32..47`, `PD(0..15)=48..63`, `PE(0..15)=64..79`; `LED_BUILTIN` is PC12 = 44, active LOW.
- Exposes `setup`, `loop`, `pinMode`, `digitalWrite`, `digitalRead`, `millis`, `delay`, approximate `delayMicroseconds`.
- Clock assumption is 22.1184 MHz HIRC. Timing may be incorrect when boot clock differs.
- 128 KB flash/16 KB RAM linker assumptions must match physical chip; inspect chip marking.
- The core is minimal; many Arduino libraries will not work yet.

## References

- Official Nu-Link Command Tool user manual: https://www.nuvoton.com/export/resource-files/UM_Nu-Link_Command_Tools_EN.pdf
- Official NUC140 V2 board manual: https://www.nuvoton.com/resource-files/UM_Nu-LB-NUC140_EN_Rev2.0.pdf
- Official NUC100 BSP: https://github.com/OpenNuvoton/NUC100BSP
- Arduino package index spec: https://docs.arduino.cc/arduino-cli/package_index_json-specification/

The new core glue code is MIT licensed. The Nuvoton command tool remains a separate, proprietary installation.
