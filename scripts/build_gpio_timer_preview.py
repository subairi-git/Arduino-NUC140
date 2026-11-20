#!/usr/bin/env python3
"""Build a self-contained, offline Windows patch for the installed NUC140 v0.1.4 core.

The preview includes only modified Arduino.h/startup.c, new ISR/timer core files,
and three test examples. Other peripheral drivers remain unchanged.
"""
from pathlib import Path
import hashlib
import zipfile

root = Path(__file__).resolve().parent.parent
out = root / "dist" / "NUC140-GPIO-Interrupt-Timer-Preview-v0.1.5.zip"
out.parent.mkdir(exist_ok=True)

files = {
    "core/Arduino.h": "hardware/nuc140/cores/arduino/Arduino.h",
    "core/startup.c": "hardware/nuc140/cores/arduino/startup.c",
    "core/wiring_interrupts.c": "hardware/nuc140/cores/arduino/wiring_interrupts.c",
    "core/wiring_timer.c": "hardware/nuc140/cores/arduino/wiring_timer.c",
    "examples/GPIO_SWINT_Interrupt/GPIO_SWINT_Interrupt.ino":
        "hardware/nuc140/libraries/Nuc140Examples/examples/GPIO_SWINT_Interrupt/GPIO_SWINT_Interrupt.ino",
    "examples/TIMER0_Blink/TIMER0_Blink.ino":
        "hardware/nuc140/libraries/Nuc140Examples/examples/TIMER0_Blink/TIMER0_Blink.ino",
    "examples/LCD_Interrupt_Timer/LCD_Interrupt_Timer.ino":
        "hardware/nuc140/libraries/NUC140LCD/examples/LCD_Interrupt_Timer/LCD_Interrupt_Timer.ino",
}
install = r'''param([switch]$NoPrompt)
$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$platform = Join-Path $env:LOCALAPPDATA "Arduino15\packages\nuc140\hardware\nuc140\0.1.4"
if (-not (Test-Path -LiteralPath $platform)) {
    throw "NUC140 Boards Manager v0.1.4 not found: $platform. Install v0.1.4 first."
}
$header = Join-Path $platform "cores\arduino\Arduino.h"
if (-not (Test-Path -LiteralPath $header) -or
    -not (Select-String -Path $header -Pattern "analogWrite" -Quiet)) {
    throw "This patch only supports integrated NUC140 core v0.1.4 with analogWrite()."
}
$backups = Join-Path $platform ".irq-timer-preview-backup-v0.1.5"
$items = @(
    @("core\Arduino.h", "cores\arduino\Arduino.h"),
    @("core\startup.c", "cores\arduino\startup.c"),
    @("core\wiring_interrupts.c", "cores\arduino\wiring_interrupts.c"),
    @("core\wiring_timer.c", "cores\arduino\wiring_timer.c"),
    @("examples\GPIO_SWINT_Interrupt\GPIO_SWINT_Interrupt.ino",
      "libraries\Nuc140Examples\examples\GPIO_SWINT_Interrupt\GPIO_SWINT_Interrupt.ino"),
    @("examples\TIMER0_Blink\TIMER0_Blink.ino",
      "libraries\Nuc140Examples\examples\TIMER0_Blink\TIMER0_Blink.ino"),
    @("examples\LCD_Interrupt_Timer\LCD_Interrupt_Timer.ino",
      "libraries\NUC140LCD\examples\LCD_Interrupt_Timer\LCD_Interrupt_Timer.ino")
)
# Validate every packaged file first, before changing the existing core.
foreach ($item in $items) {
    $from = Join-Path $here $item[0]
    if (-not (Test-Path -LiteralPath $from)) { throw "Missing packaged file: $from" }
}
New-Item -ItemType Directory -Force -Path $backups | Out-Null
foreach ($item in $items) {
    $from = Join-Path $here $item[0]
    $to = Join-Path $platform $item[1]
    $bak = Join-Path $backups $item[1]
    $absent = "$bak.absent"
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $to) | Out-Null
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $bak) | Out-Null
    if (-not (Test-Path -LiteralPath $bak) -and
        -not (Test-Path -LiteralPath $absent)) {
        if (Test-Path -LiteralPath $to) {
            Copy-Item -LiteralPath $to -Destination $bak -Force
        } else {
            Set-Content -LiteralPath $absent -Value "Absent before preview installation"
        }
    }
    Copy-Item -LiteralPath $from -Destination $to -Force
    Write-Host "Installed $($item[1])"
}
Write-Host ""
Write-Host "NUC140 IRQ/TIMER preview installed successfully." -ForegroundColor Green
Write-Host "Only Arduino v0.1.4 core files and examples were changed."
Write-Host "Backups: $backups"
Write-Host "Restart Arduino IDE; test GPIO_SWINT_Interrupt, then TIMER0_Blink, then LCD_Interrupt_Timer."
if (-not $NoPrompt) { Read-Host "Press Enter to close" | Out-Null }
'''
rollback = r'''param([switch]$NoPrompt)
$ErrorActionPreference = "Stop"
$platform = Join-Path $env:LOCALAPPDATA "Arduino15\packages\nuc140\hardware\nuc140\0.1.4"
$backups = Join-Path $platform ".irq-timer-preview-backup-v0.1.5"
if (-not (Test-Path -LiteralPath $backups)) {
    throw "No preview backup found: $backups"
}
$targets = @(
    "cores\arduino\Arduino.h",
    "cores\arduino\startup.c",
    "cores\arduino\wiring_interrupts.c",
    "cores\arduino\wiring_timer.c",
    "libraries\Nuc140Examples\examples\GPIO_SWINT_Interrupt\GPIO_SWINT_Interrupt.ino",
    "libraries\Nuc140Examples\examples\TIMER0_Blink\TIMER0_Blink.ino",
    "libraries\NUC140LCD\examples\LCD_Interrupt_Timer\LCD_Interrupt_Timer.ino"
)
foreach ($relative in $targets) {
    $to = Join-Path $platform $relative
    $bak = Join-Path $backups $relative
    if (Test-Path -LiteralPath $bak) {
        Copy-Item -LiteralPath $bak -Destination $to -Force
        Write-Host "Restored $relative"
    } elseif (Test-Path -LiteralPath "$bak.absent") {
        Remove-Item -LiteralPath $to -Force -ErrorAction SilentlyContinue
        Write-Host "Removed preview-only $relative"
    } else {
        throw "Missing backup or absent marker for: $relative"
    }
}
Write-Host "Restored previous v0.1.4 core files." -ForegroundColor Green
Write-Host "Restart Arduino IDE."
if (-not $NoPrompt) { Read-Host "Press Enter to close" | Out-Null }
'''
readme = """NUC140 GPIO Interrupt + Hardware Timer preview v0.1.5
=====================================================
For Nu-LB-NUC140 V2.0 / NUC140VE3CN, using Arduino Boards Manager v0.1.4.

INSTALLATION
1. Close Arduino IDE.
2. Extract this whole ZIP to a directory (do not run from inside ZIP).
3. Run INSTALL-IRQ-TIMER.cmd (uses PowerShell, no download needed).
4. Restart Arduino IDE.
5. Choose Tools > Board > Nu-LB-NUC140 V2.0.
6. Open File > Examples > Nuc140Examples > GPIO_SWINT_Interrupt.
   Upload with Nu-Link, monitor UART0 115200, press SW INT PB15.
7. Test Nuc140Examples > TIMER0_Blink: PC12 blinks every 500ms.
8. Test NUC140LCD > LCD_Interrupt_Timer: LCD seconds + SW count.

OPTIONAL RESTORE
Close Arduino IDE and run ROLLBACK-IRQ-TIMER.cmd.
It restores backed-up v0.1.4 core files and removes the preview files.

NEW API
attachInterrupt(digitalPinToInterrupt(PB(15)), onButton, FALLING);
detachInterrupt(digitalPinToInterrupt(PB(15)));
timerBegin(0, 1000000UL, onSecond); // TIMER0 once per second
timerStop(0); timerStart(0); timerEnd(0);

WARNING
- PREVIEW: compiles for Cortex-M0; physical GPIO IRQ and timer not yet verified.
- ISR callbacks should update volatile flags/counters ONLY. No Serial/LCD/delay.
- TIMER0..TIMER3 period uses 22.1184 MHz HIRC; accuracy depends on clock.
- Does not change existing Nu-Link, UART, ADC, Wire, SPI, LCD, RTC or PWM.
- This is a local test patch; GitHub v0.1.4 release remains unchanged.
Source: https://github.com/subairi-git/Arduino-NUC140
"""
wrappers = {
    "INSTALL-IRQ-TIMER.cmd": "@echo off\r\npowershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File \"%~dp0INSTALL-IRQ-TIMER.ps1\"\r\n",
    "ROLLBACK-IRQ-TIMER.cmd": "@echo off\r\npowershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File \"%~dp0ROLLBACK-IRQ-TIMER.ps1\"\r\n",
}
with zipfile.ZipFile(out, "w", compression=zipfile.ZIP_DEFLATED) as zipf:
    for dst, src in sorted(files.items()):
        path = root / src
        if not path.is_file():
            raise FileNotFoundError(path)
        zipf.write(path, dst)
    zipf.writestr("INSTALL-IRQ-TIMER.ps1", install)
    zipf.writestr("ROLLBACK-IRQ-TIMER.ps1", rollback)
    zipf.writestr("README.txt", readme)
    for name, body in wrappers.items():
        zipf.writestr(name, body)
    manifest = "\n".join(
        f"{hashlib.sha256((root / src).read_bytes()).hexdigest()}  {dst}"
        for dst, src in sorted(files.items())
    ) + "\n"
    zipf.writestr("SHA256SUMS.txt", manifest)

with zipfile.ZipFile(out) as zipf:
    for dst in files:
        assert dst in zipf.namelist(), dst
    assert zipf.testzip() is None
print(f"PASS: {out} ({out.stat().st_size} bytes) with {len(files)} source/example files")
