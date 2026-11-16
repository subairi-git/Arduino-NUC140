# Arduino NUC140 (EXPERIMENTAL / compile-only)

Minimal community Arduino-style core for **Nu-LB-NUC140 V2.0 (2014)** using **NUC140VE3CN, ARM Cortex-M0, 128 KB flash / 16 KB SRAM**. Please verify MCU marking before flashing.

**This is a prototype, not tested on physical hardware or with Arduino CLI yet.** The core intentionally does NOT implement automatic uploading, Serial, I2C, SPI, PWM, ADC, interrupts, or most Arduino libraries. **Do not assume official Arduino compatibility.** Only `setup()`, `loop()`, `pinMode()`, `digitalWrite()`, `digitalRead()`, `millis()`, `delay()`, and approximate `delayMicroseconds()` are provided. GPIO numbering is custom: `PA(0..15)=0..15`, `PB(0..15)=16..31`, `PC(0..15)=32..47`, `PD(0..15)=48..63`, `PE(0..15)=64..79`; `LED_BUILTIN=PC12=44` active LOW.

## Installation via Boards Manager (after you publish v0.1.1 GitHub release)
1. Create public GitHub repo `Arduino-NUC140` on your account, default branch `main`; upload **all files** here maintaining directories.
2. In GitHub repo Settings → Actions → General → Workflow permissions select **Read and write permissions**.
3. Create and push tag `v0.1.1` (Releases can create the tag, but **do not pre-create a release** because workflow creates it automatically).
4. Wait for GitHub Action **Build Arduino Boards Manager release** to finish successfully. It publishes the tarball in Releases and generates `package_nuc140_index.json` on `main` with SHA-256 and size.
5. Arduino IDE → File → Preferences → Additional Boards Manager URLs:
   `https://raw.githubusercontent.com/YOUR_USERNAME/Arduino-NUC140/main/package_nuc140_index.json`
6. Tools → Board → Boards Manager → search `NUC140` → Install. Select `Nu-LB-NUC140 V2.0 ...`.
7. Compile `File → Examples → Nuc140Examples → BlinkOnboard` using Verify. `Sketch → Export Compiled Binary` to produce firmware.

**Important:** Arduino `Upload` is deliberately unsupported until a tested Nu-Link programming command is added. Use Nuvoton's compatible official Nu-Link ICP programming application to flash an exported `.bin` at APROM base `0x00000000` only after verifying configuration. Do not overwrite LDROM/configuration words. Check the board documentation for the correct Nu-Link connection.

### Alternative manual install (development)
Copy `hardware/nuc140` into `<Arduino sketchbook>/hardware/community/nuc140` and install a suitable ARM GNU compiler separately or through the Arduino SAMD Boards package. The `platform.txt` references the Arduino managed toolchain. Boards Manager is preferred.

### Clock / stability notes
`millis()` assumes HCLK = **22.1184 MHz**, the on-chip high-speed RC clock. The actual boot clock depends on flash configuration settings. `delayMicroseconds()` is approximate. The linker assumes **128 KB APROM / 16 KB RAM**. A different MCU or changed configurations require modifying this core.

### References
- https://github.com/OpenNuvoton/NUC100BSP
- https://www.nuvoton.com/resource-files/UM_Nu-LB-NUC140_EN_Rev2.0.pdf
- https://www.nuvoton.com/resource-files/TRM_NUC130_NUC140%28CN%29_Series_EN_Rev2.05.pdf
- https://docs.arduino.cc/arduino-cli/package_index_json-specification/

MIT license for new glue code; third-party BSP not bundled.

### v0.1.1 Boards Manager packaging fix

Fixes `searching package root dir: no unique root dir in archive` by placing all
core files inside the single top-level `nuc140-0.1.1/` directory in the tarball.
The release workflow now derives the artifact name from the pushed version tag
and verifies archive structure, file size and SHA-256 before publication.
This changes packaging only; it does not add automatic Nu-Link upload or
verify compatibility with physical board hardware.
