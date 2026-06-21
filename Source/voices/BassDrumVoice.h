#pragma once

#include "Voice.h"
#include "../dsp/BandlimitedOsc.h"
#include "../dsp/PitchEnvelope.h"
#include "../dsp/Envelope.h"

namespace tr808::voices
{
// BD: a pure sine body with a downward pitch sweep and an amplitude decay (the
// punch/click comes entirely from the steep pitch modulation — no noise). Macros:
// Tone scales the pitch sweep (attack punch), Decay scales the body length, Level.
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
        return { { "freq", &deep.freq }, { "penvamt", &deep.penvAmt }, { "penvtime", &deep.penvTime },
                 { "bodydecay", &deep.bodyDecay }, { "drive", &deep.drive } };
    }

private:
    struct Deep
    {
        float freq = 52.0f, penvAmt = 180.0f, penvTime = 45.0f, bodyDecay = 300.0f, drive = 1.0f;
    } deep;

    dsp::BandlimitedOsc sine;
    dsp::PitchEnvelope  pitchEnv;
    dsp::Envelope       ampEnv;

    float amp      = 0.0f;
    float baseFreq = 52.0f;
    float driveAmt = 1.0f;
};
}
