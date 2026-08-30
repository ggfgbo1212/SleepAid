# fix_bom.ps1
# Purpose: GUI Guider screen code (setup_scr_*.c) contains Chinese string literals.
#          ARMCC V5 requires a UTF-8 BOM to compile them; without BOM it reports a
#          chain of "missing closing quote" errors. This script re-adds the BOM
#          before every build.
# Scope: only setup_scr_*.c in the generated folder (the only files with Chinese
#        string literals). Font/image/header bytes are bitmap data and must NOT get
#        a BOM - they are left untouched.
# Rule: add BOM only when missing AND file contains non-ASCII bytes (idempotent).
# Hook: Keil: Options for Target -> User -> Before Make (Run #1) -> fix_bom.bat
# NOTE: keep this file ASCII-only (PowerShell 5.1 reads .ps1 without BOM as ANSI/GBK).

$generated = Join-Path $PSScriptRoot '..\6_Middlewares\lvgl\SleepAid_UI\generated'
if (-not (Test-Path $generated)) { exit 0 }

Get-ChildItem $generated -Filter 'setup_scr_*.c' | ForEach-Object {
    $b = [System.IO.File]::ReadAllBytes($_.FullName)
    $hasBom = ($b.Length -ge 3 -and $b[0] -eq 0xEF -and $b[1] -eq 0xBB -and $b[2] -eq 0xBF)
    if ($hasBom) { return }

    # only patch files with non-ASCII (multibyte) content; pure-ASCII files are fine
    $hasNonAscii = $false
    foreach ($byte in $b) { if ($byte -ge 0x80) { $hasNonAscii = $true; break } }
    if (-not $hasNonAscii) { return }

    $out = New-Object byte[] ($b.Length + 3)
    $out[0] = 0xEF; $out[1] = 0xBB; $out[2] = 0xBF
    [System.Array]::Copy($b, 0, $out, 3, $b.Length)
    [System.IO.File]::WriteAllBytes($_.FullName, $out)
    Write-Host ("fix_bom: BOM added -> {0}" -f $_.Name)
}
