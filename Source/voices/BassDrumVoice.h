#pragma once

#include "Voice.h"
#include "../dsp/BandlimitedOsc.h"
#include "../dsp/PitchEnvelope.h"
#include "../dsp/Envelope.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/SVFilter.h"

namespace tr808::voices
{
// BD: sine body with a downward pitch sweep + amplitude decay, plus a short
// high-passed noise "click", with optional tanh drive. Deep params are the
// source of truth; macros modify (Tone scales click, Decay scales body length,
// Level trims output).
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
                 { "bodydecay", &deep.bodyDecay }, { "clicklvl", &deep.clickLvl },
                 { "clicktone", &deep.clickTone }, { "drive", &deep.drive } };
    }

private:
    struct Deep
    {
        float freq = 52.0f, penvAmt = 180.0f, penvTime = 45.0f, bodyDecay = 300.0f,
              clickLvl = 0.3f, clickTone = 0.5f, drive = 1.0f;
    } deep;

    dsp::BandlimitedOsc sine;
    dsp::PitchEnvelope  pitchEnv;
    dsp::Envelope       ampEnv;
    dsp::NoiseGen       clickNoise;
    dsp::Envelope       clickEnv;
    dsp::SVFilter       clickHpf;

    float amp        = 0.0f;
    float baseFreq   = 52.0f;
    float clickLevel = 0.0f;
    float driveAmt   = 1.0f;
};
}
