# Build a one-click Windows installer (TR-808-Synth-Setup.exe).
# Runs package.ps1 to stage the artefacts, then compiles installer\TR808Synth.iss
# with Inno Setup. Install Inno Setup 6 first: https://jrsoftware.org/isdl.php
#
# Usage:  powershell -ExecutionPolicy Bypass -File build-installer.ps1

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot

Write-Host "== Staging artefacts (package.ps1) =="
& "$root\package.ps1"

# Locate the Inno Setup compiler (ISCC.exe).
$iscc = @(
    "$env:ProgramFiles\Inno Setup 6\ISCC.exe",
    "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
) | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $iscc) { $iscc = (Get-Command ISCC.exe -ErrorAction SilentlyContinue).Source }

if (-not $iscc)
{
    Write-Warning "Inno Setup (ISCC.exe) not found."
    Write-Host    "Install Inno Setup 6 from https://jrsoftware.org/isdl.php, then re-run this script."
    Write-Host    "The plain zip is still available at dist\TR-808-Synth-Windows.zip."
    exit 1
}

Write-Host "== Compiling installer ($iscc) =="
& $iscc "$root\installer\TR808Synth.iss"
Write-Host "== Done ==`nInstaller: $root\dist\TR-808-Synth-Setup.exe"
