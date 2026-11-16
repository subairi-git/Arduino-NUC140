# Arduino NUC140 V2.0 – Experimental v0.1.3

Unofficial Arduino core for **Nu-LB-NUC140 V2.0 (2014), NUC140VE3CN** (ARM Cortex-M0). The community has **successfully compiled, programmed and run LED Blink on one NUC140VE3CN board using Windows Nu-Link**. This does not establish compatibility with all boards or sketches; the core remains experimental.

## What's new in v0.1.3

- Permanent hotfix: the Windows uploader no longer blocks on `NuLink.exe -l`, which returned `Cannot list NuLink to file` (exit -4) on a tested PC. It still requires `NuLink.exe -p` to identify **NUC140VE3CN** before changing Flash.
- Proper Arduino IDE Flash / static RAM size reporting via `recipe.size.regex` / `recipe.size.regex.data`. Includes vector table and initialized data in Flash; initialized and uninitialized static variables in RAM.
- Aligns `platform.txt`, library, and `core_version.h` versions to 0.1.3.
- Makes **Sketch → Export Compiled Binary** explicitly emit `.bin`.
- Release index builder keeps older versions *only if they already exist in the repository's package index*; the newly published 0.1.3 is first.

## Install / upgrade in Arduino IDE

Board Manager URL (use this existing URL; do not change it):

`https://raw.githubusercontent.com/subairi-git/Arduino-NUC140/main/package_nuc140_index.json`

1. Update the source repository and push tag `v0.1.3` (see `RELEASE-0.1.3-WINDOWS.md`). GitHub Actions builds the `.tar.gz`, publishes the GitHub Release, and updates `package_nuc140_index.json`.
2. In Arduino IDE → **Tools → Board → Boards Manager**, find **NUC140**, choose **0.1.3**, and update.
3. Choose **Nu-LB-NUC140 V2.0 (NUC140VE3CN)**, open **File → Examples → Nuc140Examples → BlinkOnboard**, then **Verify** and **Upload**.

## Upload with Nu-Link (Windows, SWD; no COM port)

Install Nuvoton's official **Nu-Link Command Tool** separately. This repo does not bundle NuLink.exe or its USB driver. Close any ICP or debugger application before upload, and attach exactly **one** Nu-Link.

Uploader checks the firmware size (8..131072 bytes) and plausible Cortex-M0 stack/reset vectors. Then it locates NuLink.exe, runs `NuLink.exe -p`, and requires `NUC140VE3CN` in its response. Only after that check it runs:

`NuLink -e APROM` → `NuLink -w APROM <firmware.bin>` → `NuLink -v APROM <firmware.bin>` → `NuLink -reset` → `NuLink -disconnect`.

**WARNING:** Upload **replaces the current APROM application**. Back up existing firmware if needed. The uploader deliberately does not modify LDROM, DataFlash, or configuration bits, and does not use whole-chip erase. Do not use on a different MCU model. When several Nu-Link devices are connected, unplug extras because `-l` enumeration is intentionally omitted on affected Command Tool versions.

The Nu-Link Command Tool was found on a tested Windows PC at:

`C:\Program Files (x86)\Nuvoton Tools\NuLink Command Tool\NuLink.exe`

If it is installed elsewhere, set Windows user environment variable `NUC140_NULINK_EXE` to the full file path, then restart Arduino IDE. You can check device identity without writing Flash using:

```cmd
"C:\Program Files (x86)\Nuvoton Tools\NuLink Command Tool\NuLink.exe" -p
```

### Build output / memory reporting

The size recipe uses ARM GNU `arm-none-eabi-size -A` and section-aware regexes. Flash use counts `.vectors`, `.text` (including `.rodata` in the current linker script), initializer arrays, `.ARM.exidx`, and the load image of `.data`. Static RAM counts `.data`, `.bss` and `.noinit`. This does **not** include worst-case stack or heap usage.

`Couldn't determine program size` should no longer occur when using the v0.1.3 `platform.txt`; actual Arduino IDE behaviour needs to be confirmed after updating.

### Existing experimental features and limitations

- GPIO numbering: `PA(0..15)=0..15`, `PB(0..15)=16..31`, `PC(0..15)=32..47`, `PD(0..15)=48..63`, `PE(0..15)=64..79`; `LED_BUILTIN=PC12=44`, active LOW.
- `setup`, `loop`, `pinMode`, `digitalWrite`, `digitalRead`, `millis`, `delay`, approximate `delayMicroseconds` are present.
- No compatible Arduino `Serial`, SPI, I2C, PWM, ADC or generic interrupt API yet; many Arduino libraries will not work.
- Core assumes 22.1184 MHz HIRC CPU clock. Timing may be wrong if chip clock configuration differs.
- Linker assumes 128 KB APROM / 16 KB SRAM for **NUC140VE3CN**. Verify the chip model before programming.

## References

- Nu-Link Command Tool user manual: https://www.nuvoton.com/export/resource-files/UM_Nu-Link_Command_Tools_EN.pdf
- NUC140 V2 manual: https://www.nuvoton.com/resource-files/UM_Nu-LB-NUC140_EN_Rev2.0.pdf
- NUC100 BSP: https://github.com/OpenNuvoton/NUC100BSP
- Arduino platform specifications: https://docs.arduino.cc/arduino-cli/platform-specification/

MIT license for new core glue code; the proprietary NuLink.exe stays separately installed on Windows.