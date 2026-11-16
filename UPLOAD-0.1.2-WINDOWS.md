# Patch v0.1.2 — Upload via Nu-Link Command Tool (Windows)

The Arduino IDE compile and upload path are now separate: **Verify** compiles the image, **Upload** invokes the official NuLink.exe through PowerShell to program APROM only.

## Deploy patch into existing Git clone

Extract `Arduino-NUC140-Fix-v0.1.2.zip` into your local `Arduino-NUC140` repository folder, keeping the `hardware`, `scripts` and `.github` directories, and replace files when prompted.

```cmd
cd C:\Users\USER\Downloads\Arduino-NUC140
git pull --rebase origin main
git add .
git commit -m "Add Windows Nu-Link uploader v0.1.2"
git push origin main
git tag v0.1.2
git push origin v0.1.2
```

Do **not** reuse or overwrite the existing `v0.1.1` tag. Check the GitHub Actions tab; the workflow publishes the new release and updates the index. Once updated, select **0.1.2** in Arduino Boards Manager and install/update the platform.

## First test

First run (adjust installed path if needed):

```cmd
"C:\Program Files\Nuvoton Tools\NuLink Command Tool\NuLink.exe" -l
"C:\Program Files\Nuvoton Tools\NuLink Command Tool\NuLink.exe" -p
```

The `-p` output must contain `NUC140VE3CN`. Run **Verify** first, then **Upload**. It replaces the existing APROM firmware. Do not attempt before backing up code you need to keep. A simple LED Blink uses PC12 (active LOW).

If the executable lives somewhere unexpected, set Windows user environment variable `NUC140_NULINK_EXE` to its complete path and restart Arduino IDE. No COM port selection is needed.

This patch is experimental, not hardware-tested on Windows or a real Nu-LB-NUC140; please share verbose Arduino upload logs for any error.
