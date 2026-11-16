# NUC140 Arduino core v0.1.3 - Windows-only Nu-Link Command Tool uploader.
# APROM ONLY: Never erases or writes LDROM, CONFIG, DataFlash, or the whole chip.
# Requires one connected Nu-Link and the official NuLink.exe installed separately.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$FirmwarePath
)

$ErrorActionPreference = 'Stop'

function Abort([string]$Message) {
    [Console]::Error.WriteLine("[NUC140] ERROR: $Message")
    exit 1
}

function Invoke-NuLink([string[]]$Arguments) {
    Write-Host ("[NUC140] NuLink " + ($Arguments -join ' '))
    # Native program output is preserved for diagnostics. PowerShell collects stderr too.
    $raw = & $script:NuLinkExe @Arguments 2>&1
    $code = $LASTEXITCODE
    $output = ($raw | Out-String)
    if ($output.Trim()) { Write-Host $output.TrimEnd() }
    if ($code -ne 0) { throw "NuLink returned exit code $code while running: $($Arguments -join ' ')" }
    # Some older NuLink versions print a failure but return success; fail closed.
    if ($output -match '(?i)\b(failed|failure|fail|error|cannot|can.t|not\s+found|no\s+device)\b') {
        throw "NuLink reported an error while running: $($Arguments -join ' ')"
    }
    return $output
}

try {
    $FirmwarePath = [System.IO.Path]::GetFullPath($FirmwarePath)
    if (-not [System.IO.File]::Exists($FirmwarePath)) { Abort "Firmware not found: $FirmwarePath" }
    $bytes = [System.IO.File]::ReadAllBytes($FirmwarePath)
    if ($bytes.Length -lt 8 -or $bytes.Length -gt 131072) {
        Abort "Firmware size must be 8..131072 bytes (128 KB APROM); found $($bytes.Length)."
    }
    # Basic sanity check for an ARM Cortex-M0 image linked at APROM 0x00000000.
    # First vector must point to RAM, second to a Thumb reset handler in APROM.
    $stack = [BitConverter]::ToUInt32($bytes, 0)
    $reset = [BitConverter]::ToUInt32($bytes, 4)
    if ($stack -lt 0x20000000 -or $stack -gt 0x20004000 -or
        ($reset -band 1) -ne 1 -or $reset -le 1 -or $reset -ge 131072) {
        Abort ('Unexpected Cortex-M0 vector table. SP=0x{0:X8}, RESET=0x{1:X8}. Check compiler/board selection.' -f $stack, $reset)
    }

    $candidates = New-Object System.Collections.Generic.List[string]
    if ($env:NUC140_NULINK_EXE) { [void]$candidates.Add($env:NUC140_NULINK_EXE.Trim('"')) }
    foreach ($root in @(${env:ProgramFiles}, ${env:ProgramFiles(x86)}, $env:ProgramW6432)) {
        if ($root) { [void]$candidates.Add((Join-Path $root 'Nuvoton Tools\NuLink Command Tool\NuLink.exe')) }
    }
    $pathCommand = Get-Command NuLink.exe -ErrorAction SilentlyContinue
    if ($pathCommand -and $pathCommand.Source) { [void]$candidates.Add($pathCommand.Source) }
    $script:NuLinkExe = $null
    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            $script:NuLinkExe = (Resolve-Path -LiteralPath $candidate).Path
            break
        }
    }
    if (-not $script:NuLinkExe) {
        Abort 'NuLink.exe not found. Set Windows environment variable NUC140_NULINK_EXE to the full path of NuLink.exe, then restart Arduino IDE.'
    }
    Write-Host "[NUC140] Command tool: $script:NuLinkExe"
    Write-Host "[NUC140] Firmware: $FirmwarePath ($($bytes.Length) bytes)"

    # NuLink.exe -l may fail with exit -4 even when -p reads the MCU.
    # Require exactly one Nu-Link connected; use -p to validate the part.

    # Official -p reports MCU part; do not erase unless the expected device is detected.
    $part = Invoke-NuLink -Arguments @('-p')
    if ($part -notmatch '(?i)\bNUC140VE3CN\b') {
        Abort 'Connected target does not report NUC140VE3CN. No flash erased; check MCU marking and Nu-Link connection.'
    }
    Write-Host '[NUC140] Target validated: NUC140VE3CN.'
    Write-Host '[NUC140] WARNING: Existing APROM application will be replaced.'

    # Follow official documented APROM-only sequence, then verify and reset.
    $null = Invoke-NuLink -Arguments @('-e', 'APROM')
    $null = Invoke-NuLink -Arguments @('-w', 'APROM', $FirmwarePath)
    $null = Invoke-NuLink -Arguments @('-v', 'APROM', $FirmwarePath)
    $null = Invoke-NuLink -Arguments @('-reset')
    $null = Invoke-NuLink -Arguments @('-disconnect')
    Write-Host '[NUC140] SUCCESS: APROM programmed, verified and reset using Nu-Link.'
    exit 0
} catch {
    [Console]::Error.WriteLine("[NUC140] Upload failed: $($_.Exception.Message)")
    if ($script:NuLinkExe) {
        try { & $script:NuLinkExe -disconnect 2>&1 | Out-Null } catch { }
    }
    exit 1
}
