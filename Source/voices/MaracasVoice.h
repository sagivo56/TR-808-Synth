#pragma once

#include "Voice.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/SVFilter.h"
#include "../dsp/Envelope.h"

namespace tr808::voices
{
// MA: high-passed white noise, very short decay. Deep: HPF, Decay. Macro: Level.
class MaracasVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

    std::vector<DeepRef> deepRefs() override
    {
        return { { "hpf", &deep.hpf }, { "decaytime", &deep.decayTime } };
    }

private:
    struct Deep { float hpf = 6000.0f, decayTime = 30.0f; } deep;

    dsp::NoiseGen noise;
    dsp::SVFilter hpf;
    dsp::Envelope env;
    float amp = 0.0f;
};
}
