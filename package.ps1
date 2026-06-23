# Package the TR-808 plugin for installing on another Windows machine.
# Produces dist\TR-808-Synth-Windows.zip with the VST3 + Standalone, built with
# a statically-linked runtime (no VC++ redistributable needed on the target).
#
# Usage:  powershell -ExecutionPolicy Bypass -File package.ps1

$ErrorActionPreference = "Stop"
$root      = $PSScriptRoot
$buildDir  = Join-Path $root "dist-build"
$distDir   = Join-Path $root "dist"
$stageDir  = Join-Path $distDir "TR-808 Synth"

Write-Host "== Configuring distribution build (static runtime) =="
cmake -S $root -B $buildDir -DTR808_STATIC_RUNTIME=ON -DTR808_BUILD_TESTS=OFF

Write-Host "== Building Release (VST3 + Standalone) =="
cmake --build $buildDir --config Release --target TR808Synth_VST3
cmake --build $buildDir --config Release --target TR808Synth_Standalone

$art = Join-Path $buildDir "TR808Synth_artefacts\Release"
$vst3 = Join-Path $art "VST3\TR-808 Synth.vst3"
$exe  = Join-Path $art "Standalone\TR-808 Synth.exe"

Write-Host "== Staging =="
if (Test-Path $distDir) { Remove-Item $distDir -Recurse -Force }
New-Item -ItemType Directory -Force -Path $stageDir | Out-Null
Copy-Item $vst3 (Join-Path $stageDir "TR-808 Synth.vst3") -Recurse
Copy-Item $exe  (Join-Path $stageDir "TR-808 Synth.exe")

@"
TR-808 Synth - install

VST3 (for your DAW):
  Copy "TR-808 Synth.vst3" into:
     C:\Program Files\Common Files\VST3\
  Then rescan plugins in your DAW.

Standalone (no DAW needed):
  Just run "TR-808 Synth.exe". Pick your audio device in its Options menu.

No extra runtime needs to be installed - the runtime is built in.
"@ | Out-File -Encoding utf8 (Join-Path $stageDir "INSTALL.txt")

$zip = Join-Path $distDir "TR-808-Synth-Windows.zip"
Compress-Archive -Path "$stageDir\*" -DestinationPath $zip -Force
Write-Host "== Done ==`nZip: $zip"
