#pragma once

#include "Voice.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/SVFilter.h"
#include "../dsp/Envelope.h"

namespace tr808::voices
{
// MA: high-passed white noise with a very short decay. Macro: Level.
class MaracasVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

private:
    dsp::NoiseGen noise;
    dsp::SVFilter hpf;
    dsp::Envelope env;
    float amp = 0.0f;
};
}
