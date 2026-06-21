#pragma once

#include "Voice.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/SVFilter.h"
#include "../dsp/Envelope.h"
#include <array>

namespace tr808::voices
{
// CP: band-passed noise gated by a multi-pulse envelope (several fast bursts +
// a room tail). Deep: BP centre/Q, pulse count, spacing, tail decay. Macro: Level.
class ClapVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

    std::vector<DeepRef> deepRefs() override
    {
        return { { "bpfreq", &deep.bpFreq }, { "bpq", &deep.bpQ }, { "npulses", &deep.nPulses },
                 { "spacing", &deep.spacing }, { "taildecay", &deep.tailDecay }, { "attack", &deep.attack } };
    }

private:
    static constexpr int maxPulses = 6;

    struct Deep
    {
        float bpFreq = 1000.0f, bpQ = 1.3f, nPulses = 3.0f, spacing = 10.0f, tailDecay = 120.0f,
              attack = 0.1f;
    } deep;

    dsp::NoiseGen noise;
    dsp::SVFilter bp;
    dsp::Envelope pulseEnv;
    dsp::Envelope tailEnv;

    float amp = 0.0f;
    int   counter = 0;
    int   pulseIndex = 0;
    int   numPulses = 3;
    std::array<int, maxPulses> pulseOffsets { {} };
};
}
