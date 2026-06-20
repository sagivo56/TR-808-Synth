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
// high-passed noise "click" transient. Macros: Tone (click amount/length),
// Decay (body length), Level.
class BassDrumVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

private:
    dsp::BandlimitedOsc sine;
    dsp::PitchEnvelope  pitchEnv;
    dsp::Envelope       ampEnv;
    dsp::NoiseGen       clickNoise;
    dsp::Envelope       clickEnv;
    dsp::SVFilter       clickHpf;

    float amp        = 0.0f;
    float baseFreq   = 52.0f;
    float clickLevel = 0.0f;
};
}
