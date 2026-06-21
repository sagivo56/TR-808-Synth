#pragma once

#include "Voice.h"
#include "../dsp/SchmittOsc.h"
#include "../dsp/SVFilter.h"
#include "../dsp/Envelope.h"

namespace tr808::voices
{
// CB: two squares mixed and band-passed, sharp attack, medium decay.
// Deep: Osc1/Osc2 freq, mix, BP centre/Q, decay. Macro: Level.
class CowbellVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

    std::vector<DeepRef> deepRefs() override
    {
        return { { "o1freq", &deep.o1freq }, { "o2freq", &deep.o2freq }, { "oscmix", &deep.oscmix },
                 { "bpfreq", &deep.bpFreq }, { "bpq", &deep.bpQ }, { "decaytime", &deep.decayTime },
                 { "attack", &deep.attack } };
    }

private:
    struct Deep
    {
        float o1freq = 540.0f, o2freq = 800.0f, oscmix = 0.5f,
              bpFreq = 2640.0f, bpQ = 0.8f, decayTime = 400.0f, attack = 1.0f;
    } deep;

    dsp::SchmittOsc o1, o2;       // 808 square oscillators (circuit model)
    dsp::SVFilter   bp;
    dsp::Envelope   env;
    float amp = 0.0f;
    float mix = 0.5f;
};
}
