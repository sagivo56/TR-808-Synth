# TR-808 Synth

A TR-808-style drum-machine plugin where **every sound is synthesised in real time
(no samples)** — circuit-behavioural emulation of the original 808 voices. JUCE 8 /
CMake / C++17. Builds as **VST3 + Standalone** on Windows (plus **AU** on macOS).

> Project lives at `C:\dev\808-app` (kept outside OneDrive so the build tree and the
> fetched JUCE source aren't synced to the cloud).

**v1.0 — feature complete.** Passes `pluginval` strictness 10. Offline tests:
dsp 46/46, voice 87/87, seq 33/33.

### Features
- 16 circuit-modelled 808 voices + a melodic **BD Bass** (tonal piano-roll, scale/
  root/length; can duck the regular BD).
- Sequencer: 16/32 steps, 4/4 & 3/4, triplets, **A–D variations** + cycle, flam,
  probability, song chain, per-step accent; copy a variation onto another.
- Parallel **Reverb (Lexicon-flavoured FDN) + ping-pong Delay** with per-voice sends.
- Mixer: per-voice level/tone/decay/tune/pan, mute/solo, master drive, stereo ⇄ 16
  multi-out.
- PERFORM / EDIT / FX views, factory + user presets, `SPACE` = transport.

## Prerequisites (Windows)

- **CMake** and **Visual Studio** (MSVC) with the *Desktop development with C++* workload.
- **git**. JUCE 8.0.13 is fetched automatically on first configure (into `build/_deps`).

## Build

```powershell
cmake -B build
cmake --build build --config Release --parallel
```

Keep the machine awake during the first configure (it downloads + builds JUCE). Artefacts:

```
build\TR808Synth_artefacts\Release\VST3\TR-808 Synth.vst3
build\TR808Synth_artefacts\Release\Standalone\TR-808 Synth.exe
```

## Run / test

- **Standalone:** run the `.exe` (choose an audio device in Options); `SPACE` plays/stops.
- **Tests:** build + run the `dsp_tests`, `voice_tests`, `seq_tests` console apps
  (each exits non-zero on failure).

### pluginval

`pluginval.exe` is a GUI-subsystem exe and its path contains a space — run it so
PowerShell waits and the path is one quoted argument:

```powershell
$pv   = (Get-ChildItem C:\dev -Recurse -Filter pluginval.exe | Select -First 1).FullName
$vst3 = "C:\dev\808-app\build\TR808Synth_artefacts\Release\VST3\TR-808 Synth.vst3"
Start-Process $pv -NoNewWindow -Wait -ArgumentList "--strictness-level 10 --validate `"$vst3`""
```

## Package & install

```powershell
powershell -ExecutionPolicy Bypass -File package.ps1          # -> dist\TR-808-Synth-Windows.zip
powershell -ExecutionPolicy Bypass -File build-installer.ps1  # -> dist\TR-808-Synth-Setup.exe (needs Inno Setup 6)
```

Both link the MSVC runtime statically, so the result runs on any Windows machine with
**no** VC++ redistributable. To install by hand: copy `TR-808 Synth.vst3` to
`C:\Program Files\Common Files\VST3\` and rescan plugins in your DAW (it appears as an
**instrument / VSTi**).

## Layout

```
Source/dsp/     DSP blocks (oscillators, resonators, reverb, delay, …)
Source/voices/  the 16 drum voices + melodic bass
Source/engine/  VoiceManager, Sequencer, Mixer, presets, parameter tables
Source/ui/      look & feel, knobs, step-sequencer view
Source/         PluginProcessor / PluginEditor
tests/          offline DSP / voice / sequencer harnesses
package.ps1, build-installer.ps1, installer/   distribution
```
