// Voice-engine tests for M2.
//
// Triggers each of the 16 voices through the VoiceManager and checks that it
// produces bounded, non-silent, decaying output; that the GM note map routes to
// the right voice; that triggering is sample-accurate; and that the closed hat
// chokes the open hat. Console app: exits non-zero on any failure.

#include <juce_audio_basics/juce_audio_basics.h>

#include "engine/VoiceManager.h"
#include "engine/VoiceDefs.h"
#include "engine/Mixer.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

using namespace tr808;

namespace
{
int g_total = 0;
int g_failed = 0;

void check (bool cond, const std::string& name, const std::string& detail = {})
{
    ++g_total;
    if (cond) { std::cout << "  [pass] " << name << "\n"; }
    else      { ++g_failed; std::cout << "  [FAIL] " << name; if (! detail.empty()) std::cout << "   (" << detail << ")"; std::cout << "\n"; }
}

constexpr double kSampleRate = 44100.0;
constexpr int    kBlock      = 512;

double rms (const std::vector<float>& v, int start, int count)
{
    double acc = 0.0; int n = 0;
    for (int i = start; i < start + count && i < (int) v.size() && i >= 0; ++i) { acc += (double) v[(size_t) i] * v[(size_t) i]; ++n; }
    return n > 0 ? std::sqrt (acc / n) : 0.0;
}

bool allFinite (const std::vector<float>& v, float bound)
{
    for (float x : v) if (! std::isfinite (x) || std::abs (x) > bound) return false;
    return true;
}

// Trigger one voice directly and render it for 'seconds' into a mono vector.
std::vector<float> renderVoice (VoiceManager& vm, int idx, float vel, double seconds)
{
    vm.reset();
    vm.noteOn (idx, vel, false);

    const int totalBlocks = (int) std::ceil (seconds * kSampleRate / kBlock);
    juce::AudioBuffer<float> buf (2, kBlock);
    juce::MidiBuffer empty;

    std::vector<float> out;
    out.reserve ((size_t) (totalBlocks * kBlock));
    for (int b = 0; b < totalBlocks; ++b)
    {
        vm.process (buf, empty);
        const auto* p = buf.getReadPointer (0);
        for (int i = 0; i < kBlock; ++i) out.push_back (p[i]);
    }
    return out;
}
} // namespace

static void testEachVoice (VoiceManager& vm)
{
    std::cout << "Per-voice synthesis\n";
    for (int i = 0; i < numVoices; ++i)
    {
        const auto& spec = voiceSpecs()[(size_t) i];
        auto out = renderVoice (vm, i, 1.0f, 5.0);

        const double head = rms (out, 0, 4410);                            // first 100 ms
        const double tail = rms (out, (int) out.size() - 8820, 8820);      // last 200 ms

        const std::string nm = spec.name;
        check (allFinite (out, 8.0f), nm + ": output finite & bounded");
        check (head > 1.0e-3,         nm + ": produces sound", "head_rms=" + std::to_string (head));
        check (tail < head * 0.5,     nm + ": decays over time", "head=" + std::to_string (head) + " tail=" + std::to_string (tail));
        check (! vm.isVoiceActive (i), nm + ": becomes inactive within 5 s");
    }
}

static void testGmMapping()
{
    std::cout << "GM note mapping\n";
    check (gmNoteToVoice (36) == BD, "note 36 -> Bass Drum");
    check (gmNoteToVoice (38) == SD, "note 38 -> Snare");
    check (gmNoteToVoice (42) == CH, "note 42 -> Closed Hat");
    check (gmNoteToVoice (46) == OH, "note 46 -> Open Hat");
    check (gmNoteToVoice (49) == CY, "note 49 -> Cymbal");
    check (gmNoteToVoice (75) == CL, "note 75 -> Claves");
    check (gmNoteToVoice (1)  == -1, "unmapped note -> none");
}

static void testSampleAccurateTrigger (VoiceManager& vm)
{
    std::cout << "Sample-accurate MIDI trigger\n";
    vm.reset();

    juce::MidiBuffer midi;
    midi.addEvent (juce::MidiMessage::noteOn (10, 36, (juce::uint8) 100), 100);   // BD at sample 100

    juce::AudioBuffer<float> buf (2, kBlock);
    vm.process (buf, midi);

    std::vector<float> out (kBlock);
    const auto* p = buf.getReadPointer (0);
    for (int i = 0; i < kBlock; ++i) out[(size_t) i] = p[i];

    check (vm.isVoiceActive (BD), "MIDI note 36 triggered the Bass Drum");
    check (rms (out, 0, 100) < 1.0e-7, "silent before the trigger sample");
    check (rms (out, 100, kBlock - 100) > 1.0e-3, "sound starts at the trigger sample");
}

static void testHatChoke (VoiceManager& vm)
{
    std::cout << "Hi-hat choke group\n";
    vm.reset();

    juce::AudioBuffer<float> buf (2, kBlock);
    juce::MidiBuffer empty;

    vm.noteOn (OH, 1.0f, false);
    vm.process (buf, empty);                       // ~12 ms
    check (vm.isVoiceActive (OH), "open hat is ringing");

    vm.noteOn (CH, 1.0f, false);                   // should choke OH
    for (int b = 0; b < 3; ++b) vm.process (buf, empty);   // ~35 ms
    check (! vm.isVoiceActive (OH), "closed hat chokes the open hat");
    check (vm.isVoiceActive (CH), "closed hat itself is sounding");
}

static void testMixer()
{
    std::cout << "Mixer (pan / mute / solo / master)\n";
    Mixer mix; mix.prepare (kSampleRate, kBlock);

    auto centre = mix.gainsFor (BD);
    check (std::abs (centre.panL - centre.panR) < 1.0e-4 && std::abs (centre.panL - 0.70710678f) < 1.0e-3,
           "centre pan is equal-power (~-3 dB both)");

    mix.setPan (BD, -1.0f);
    auto left = mix.gainsFor (BD);
    check (left.panL > 0.99f && left.panR < 0.01f, "hard left = L only");
    mix.setPan (BD, 1.0f);
    auto right = mix.gainsFor (BD);
    check (right.panR > 0.99f && right.panL < 0.01f, "hard right = R only");

    mix.setMute (BD, true);
    check (mix.gainsFor (BD).gate == 0.0f, "mute gates the voice");

    Mixer solo; solo.prepare (kSampleRate, kBlock);
    solo.setSolo (SD, true);
    check (solo.gainsFor (SD).gate == 1.0f && solo.gainsFor (BD).gate == 0.0f, "solo isolates the soloed voice");

    // Master gentle limiter: a very loud buffer is bounded by the tanh.
    juce::AudioBuffer<float> buf (2, kBlock);
    for (int ch = 0; ch < 2; ++ch) { auto* p = buf.getWritePointer (ch); for (int i = 0; i < kBlock; ++i) p[i] = 5.0f; }
    solo.setMasterDrive (1.0f);
    solo.processMaster (buf);
    float steady = 0.0f, peak = 0.0f;
    for (int ch = 0; ch < 2; ++ch)
    {
        auto* p = buf.getReadPointer (ch);
        for (int i = 0; i < kBlock; ++i)
        {
            peak = std::max (peak, std::abs (p[i]));
            if (i >= 200) steady = std::max (steady, std::abs (p[i]));   // past the oversampler settling
        }
    }
    check (steady <= 1.001f, "master limiter clamps the sustained level to ~1", "steady=" + std::to_string (steady));
    check (std::isfinite (peak) && peak < 1.5f, "master bus stays bounded (brief filter overshoot ok)", "peak=" + std::to_string (peak));
}

static void testMultiOutRouting (VoiceManager& vm)
{
    std::cout << "Multi-out routing\n";
    Mixer mix; mix.prepare (kSampleRate, kBlock);
    mix.setPan (BD, -1.0f);                       // hard left

    vm.reset();
    std::vector<TriggerEvent> ev { { 0, BD, 1.0f, false } };
    juce::AudioBuffer<float> main (2, kBlock);
    std::vector<float> auxBD ((size_t) kBlock, 0.0f);
    std::array<float*, numVoices> aux {};
    aux[(size_t) BD] = auxBD.data();

    vm.renderEvents (main, ev, mix, aux.data());

    std::vector<float> L (main.getReadPointer (0), main.getReadPointer (0) + kBlock);
    std::vector<float> R (main.getReadPointer (1), main.getReadPointer (1) + kBlock);

    check (rms (L, 0, kBlock) > 1.0e-3, "master L carries the BD (panned left)");
    check (rms (R, 0, kBlock) < rms (L, 0, kBlock) * 0.05, "master R near-silent for hard-left voice");
    check (rms (auxBD, 0, kBlock) > 1.0e-3, "BD aux send carries the voice");
}

int main()
{
    std::cout << "=== TR-808 voice tests (sr=" << kSampleRate << ") ===\n";

    VoiceManager vm;
    vm.prepare (kSampleRate, kBlock);

    testEachVoice (vm);
    testGmMapping();
    testSampleAccurateTrigger (vm);
    testHatChoke (vm);
    testMixer();
    testMultiOutRouting (vm);

    std::cout << "\n" << (g_total - g_failed) << "/" << g_total << " checks passed.\n";
    if (g_failed > 0) { std::cout << "*** " << g_failed << " FAILED ***\n"; return 1; }
    std::cout << "ALL PASSED\n";
    return 0;
}
