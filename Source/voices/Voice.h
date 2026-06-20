#pragma once

#include <algorithm>
#include <cmath>

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

protected:
    // Per-hit output amplitude from level macro, velocity and accent.
    float triggerAmp (float velocity, bool accent) const noexcept
    {
        const float v = std::clamp (velocity, 0.0f, 1.0f);
        return macros.level * v * (accent ? 1.3f : 1.0f);
    }

    double      sampleRate = 44100.0;
    VoiceMacros macros;
};
}
