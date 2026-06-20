#pragma once

#include "Voice.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/SVFilter.h"
#include "../dsp/Envelope.h"
#include <array>

namespace tr808::voices
{
// CP: band-passed noise gated by a multi-pulse envelope — three fast bursts a
// few ms apart (the classic clap "spread") followed by a decaying room tail.
// Macro: Level.
class ClapVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

private:
    dsp::NoiseGen noise;
    dsp::SVFilter bp;
    dsp::Envelope pulseEnv;     // fast bursts
    dsp::Envelope tailEnv;      // room tail

    float amp = 0.0f;
    int   counter = 0;
    int   pulseIndex = 0;
    std::array<int, 3> pulseOffsets { { 0, 0, 0 } };
};
}
