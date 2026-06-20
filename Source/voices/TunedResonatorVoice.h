#pragma once

#include "Voice.h"
#include "../dsp/ResonatorBT.h"

namespace tr808::voices
{
// RS (rim shot) and CL (claves): a short, high-pitched impulse-excited
// resonator. Deep: Tune, Decay. Macro: Level.
class TunedResonatorVoice : public Voice
{
public:
    void setConfig (float frequency, float decaySeconds) noexcept
    {
        deep.freq      = frequency;
        deep.decayTime = decaySeconds * 1000.0f;
    }

    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

    std::vector<DeepRef> deepRefs() override
    {
        return { { "freq", &deep.freq }, { "decaytime", &deep.decayTime } };
    }

private:
    struct Deep { float freq = 1700.0f, decayTime = 60.0f; } deep;

    dsp::ResonatorBT res;
    float amp = 0.0f;
};
}
