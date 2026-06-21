#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace tr808::dsp {}   // fwd: blocks live here

namespace tr808::voices
{
namespace dsp = ::tr808::dsp;

//==============================================================================
// Normalised 0..1 "panel" macros. Each voice interprets only the fields it
// uses (see VoiceSpec). Level is linear gain; the rest are mapped to musical
// ranges inside each voice.
//==============================================================================
struct VoiceMacros
{
    float level  = 0.8f;
    float tone   = 0.5f;
    float decay  = 0.5f;
    float snappy = 0.5f;
    float tune   = 0.5f;
};

// 0..1 -> [lo, hi] linear / exponential helpers.
inline float mapLin (float t, float lo, float hi) noexcept { return lo + (hi - lo) * std::clamp (t, 0.0f, 1.0f); }
inline float mapExp (float t, float lo, float hi) noexcept { return lo * std::pow (hi / lo, std::clamp (t, 0.0f, 1.0f)); }

// Macro-as-modifier helpers, neutral at 0.5:
//  centeredScale -> multiplier in [1/factor, factor]  (0.5 => 1.0)
//  centeredPitch -> frequency multiplier of +/- 'semis' semitones (0.5 => 1.0)
inline float centeredScale (float macro, float factor) noexcept { return mapExp (macro, 1.0f / factor, factor); }
inline float centeredPitch (float macro, float semis)  noexcept { return std::pow (2.0f, (std::clamp (macro, 0.0f, 1.0f) - 0.5f) * 2.0f * semis / 12.0f); }

// One automatable Deep parameter, exposed by a voice: the parameter-id suffix
// and a pointer to the float the processor writes the APVTS value into.
struct DeepRef { std::string suffix; float* ptr; };

//==============================================================================
// Base class for all 16 voices. Pure DSP: depends only on the /dsp blocks, no
// JUCE/UI — so each voice can be triggered and rendered in an offline test.
// renderAdd() *adds* mono output (the VoiceManager sums voices then spreads to
// stereo).
//==============================================================================
class Voice
{
public:
    virtual ~Voice() = default;

    virtual void prepare (double sr, int /*maxBlock*/) { sampleRate = sr; }
    virtual void reset() = 0;

    void setMacros (const VoiceMacros& m) noexcept { macros = m; }

    virtual void trigger (float velocity, bool accent) = 0;
    virtual void renderAdd (float* mono, int numSamples) = 0;
    virtual bool isActive() const = 0;

    virtual void choke() { reset(); }     // hard stop by default; metallic voices override with a fast fade

    // Deep-edit params (M3): a voice lists its automatable per-stage params here;
    // the processor writes APVTS values straight into the referenced floats.
    virtual std::vector<DeepRef> deepRefs() { return {}; }

protected:
    // Per-hit output amplitude from level macro and velocity. The accent boost
    // is applied to the velocity upstream (VoiceManager), so it stays adjustable.
    float triggerAmp (float velocity, bool /*accent*/) const noexcept
    {
        return macros.level * std::max (0.0f, velocity);
    }

    double      sampleRate = 44100.0;
    VoiceMacros macros;
};
}
