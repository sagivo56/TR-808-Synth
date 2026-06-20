#pragma once

#include "Voice.h"
#include "../dsp/ResonatorBT.h"
#include "../dsp/PitchEnvelope.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/Envelope.h"
#include "../dsp/SVFilter.h"

namespace tr808::voices
{
// Toms (LT/MT/HT) and congas (LC/MC/HC): a tuned resonator body with a short
// downward pitch sweep and a tiny attack-noise click. Congas are just a
// higher-pitched, shorter-decay configuration. Macros: Tuning, Level.
class TomCongaVoice : public Voice
{
public:
    void setConfig (float baseFrequency, float decaySeconds) noexcept
    {
        baseFreq = baseFrequency;
        decaySec = decaySeconds;
    }

    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

private:
    dsp::ResonatorBT   res;
    dsp::PitchEnvelope pitchEnv;
    dsp::NoiseGen      attackNoise;
    dsp::Envelope      attackEnv;
    dsp::SVFilter      attackHpf;

    float amp         = 0.0f;
    float baseFreq    = 120.0f;
    float decaySec    = 0.5f;
    float fBase       = 120.0f;
    float attackLevel = 0.3f;
};
}
