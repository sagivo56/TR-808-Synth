# TR-808 Synth

A TR-808-style drum-machine plugin where **every sound is synthesised in real time
(no samples)**. Built with JUCE + CMake (C++17). Targets **VST3** and **Standalone**
on Windows, plus **AU** when built on macOS.

> Project lives at `C:\dev\808-app` (kept outside OneDrive so the build tree and the
> fetched JUCE source aren't synced to the cloud).

---

## Status

| Milestone | Description | State |
|-----------|-------------|-------|
| **M0** | Scaffold: CMake+JUCE, VST3/AU/Standalone targets, APVTS skeleton, silent output | ✅ **done** — builds (VST3+Standalone), passes `pluginval` strictness 10 |
| **M1** | DSP building blocks | ✅ **done** — 8 blocks in `Source/dsp`, 36/36 offline checks pass |
| **M2** | Voice engine (16 voices) | ✅ **done** — GM trigger, Macro params, hat choke; voice_tests 77/77, pluginval 10 |
| **M3** | Deep-edit params | ✅ **done** — all per-stage params in APVTS (automatable), macros = modifiers; pluginval 10 |
| **M4** | Sequencer | ✅ **done** — 16-step, host-synced sample-accurate, accent/flam/swing/AB/mute-solo; seq_tests 18/18, pluginval 10 |
| **M5** | Mixer & routing | ✅ **done** — per-voice pan/mute/solo, master drive+limiter, stereo ⇄ 16 multi-out + fallback; pluginval 10 |
| M6 | UI | ⬜ |
| M7 | Presets & polish | ⬜ |

---

## Prerequisites (Windows)

Installed and verified on this machine:

- **CMake 4.4.0**
- **Visual Studio Community 2026** (v18) with the **“Desktop development with C++”** workload
  (MSVC 14.51). CMake auto-detects the **“Visual Studio 18 2026”** generator.
- **git**

To set the toolchain up from scratch on another machine: install CMake (`winget install
--id Kitware.CMake -e`) and Visual Studio (or Build Tools) **with the C++ workload**
(`--add Microsoft.VisualStudio.Workload.VCTools`). Open a new terminal afterwards so
`cmake` is on `PATH`.

---

## Build

From the project root (`C:\dev\808-app`):

```powershell
# Configure — let CMake auto-detect the VS generator (here: "Visual Studio 18 2026")
cmake -B build -A x64

# Build Release
cmake --build build --config Release --parallel
```

First configure downloads JUCE 8.0.13 (via FetchContent) into `build/_deps` and builds
`juceaide` — a few minutes. **Keep the machine awake during the first configure/build:**
if it sleeps, the detached build process is killed and `build/` is left half-written
(delete `build/` and re-run if that happens).

### Artefacts

```
build\TR808Synth_artefacts\Release\VST3\TR-808 Synth.vst3
build\TR808Synth_artefacts\Release\Standalone\TR-808 Synth.exe
```

`COPY_PLUGIN_AFTER_BUILD` is **off** (so the build never needs admin). To test the VST3
in a DAW, copy the `.vst3` to your VST3 folder (typically
`C:\Program Files\Common Files\VST3`, needs admin) or point your DAW's scan path at the
build folder.

---

## Validate (pluginval)

`pluginval.exe` lives at `C:\dev\tools\pluginval\pluginval.exe`. It's a **GUI-subsystem**
exe, so a bare `& pluginval ...` call returns immediately without waiting and the path
(which contains a space) gets split. Run it like this so PowerShell waits and the path is
quoted as a single argument:

```powershell
$pv   = "C:\dev\tools\pluginval\pluginval.exe"
$vst3 = "C:\dev\808-app\build\TR808Synth_artefacts\Release\VST3\TR-808 Synth.vst3"
$proc = Start-Process $pv -NoNewWindow -Wait -PassThru `
  -ArgumentList "--validate-in-process --strictness-level 10 --validate `"$vst3`"" `
  -RedirectStandardOutput pv.log -RedirectStandardError pv_err.log
"exit=$($proc.ExitCode)"   # 0 = pass
```

M0 result: **strictness 10 → SUCCESS** (all tests pass).

---

## DSP tests (M1)

The `/dsp` blocks have an offline harness (`tests/DspTests.cpp`) built as the
`dsp_tests` console app. It renders each block and checks it numerically and
spectrally (FFT) — including that the PolyBLEP square is band-limited. Run:

```powershell
cmake --build build --config Release --target dsp_tests
& "build\dsp_tests_artefacts\Release\dsp_tests.exe"   # exits non-zero on any failure
```

M1 result: **36/36 checks pass**.

The voice engine (M2) has its own harness (`tests/VoiceTests.cpp` → `voice_tests`):
it triggers all 16 voices and checks output/decay, the GM note map, sample-accurate
triggering and the hi-hat choke.

```powershell
cmake --build build --config Release --target voice_tests
& "build\voice_tests_artefacts\Release\voice_tests.exe"
```

M2 result: **77/77 checks pass**; pluginval strictness 10 → SUCCESS on the
synthesizing plugin.

The sequencer (M4) has `tests/SeqTests.cpp` → `seq_tests`: PPQ step timing
(sample-accuracy), no double-fire across blocks, internal transport, mute/solo,
swing and a state round-trip.

```powershell
cmake --build build --config Release --target seq_tests
& "build\seq_tests_artefacts\Release\seq_tests.exe"
```

M4 result: **seq_tests 18/18**; pluginval strictness 10 → SUCCESS with the
sequencer integrated.

---

## M0 acceptance checklist

- [x] Configures and builds cleanly (VST3 + Standalone).
- [x] `pluginval --strictness-level 10` passes (editor, audio, state, automation, buses).
- [x] Output is silence (`processBlock` clears the buffer; pluginval audio tests clean).
- [ ] VST3 loads in a DAW — *manual check (optional); pluginval covers load/process/state/automation*.

---

## Layout

```
CMakeLists.txt
Source/
  PluginProcessor.{h,cpp}   AudioProcessor + APVTS, MIDI in, stereo out, silent (M0)
  PluginEditor.{h,cpp}      Minimal resizable window (replaced by full UI in M6)
  params/
    ParameterIDs.h          Stable string IDs for every parameter
    ParameterLayout.{h,cpp} Builds the APVTS layout (UI-independent, testable)
```
