#pragma once

#include "Voice.h"
#include "../dsp/ResonatorBT.h"

namespace tr808::voices
{
// BD — service-manual circuit model: a bridged-T resonator (decaying sine, decay
// set by Q/feedback). On trigger it rings at a "punch" multiple of the inherent
// frequency for the first half-cycle, then drops to the inherent frequency and a
// retrigger pulse re-excites it (the Q41-Q43 trick). Macros: Tone scales the
// punch, Decay scales the ring, Level.
class BassDrumVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

    std::vector<DeepRef> deepRefs() override
    {
        return { { "freq", &deep.freq }, { "bodydecay", &deep.decay }, { "punch", &deep.punch },
                 { "retrig", &deep.retrig }, { "sustain", &deep.sustain }, { "drive", &deep.drive } };
    }

private:
    struct Deep
    {
        float freq = 55.0f, decay = 320.0f, punch = 2.0f, retrig = 0.5f, sustain = 0.0f, drive = 1.0f;
    } deep;

    dsp::ResonatorBT res;

    float amp        = 0.0f;
    float baseFreq   = 55.0f;
    float driveAmt   = 1.0f;
    float retrigAmt  = 0.5f;
    int   switchSample = 0;
    int   sampleCount  = 0;
    bool  switched     = true;
};
}
