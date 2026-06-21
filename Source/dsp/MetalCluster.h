#pragma once

#include "SchmittOsc.h"
#include <array>

namespace tr808::dsp
{
//==============================================================================
// Six inharmonic Schmitt-trigger (circuit-modeled) square oscillators summed
// together — the shared metallic source for cymbal, open hat and closed hat.
// Each voice then shapes this with its own HPF/BP + envelope.
//==============================================================================
class MetalCluster
{
public:
    static constexpr int numOscillators = 6;

    void prepare (double sampleRate);
    void reset();                                          // reset all phases

    void setFrequency (int index, float hz) noexcept;
    void setFrequencies (const std::array<float, numOscillators>& f) noexcept;

    float processSample() noexcept;

private:
    std::array<SchmittOsc, numOscillators> oscs;
    std::array<float, numOscillators> freqs { { 205.3f, 304.4f, 369.6f, 522.7f, 540.0f, 800.0f } };
};
}
