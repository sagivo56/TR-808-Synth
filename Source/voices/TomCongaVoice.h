#pragma once

#include "Voice.h"
#include "../dsp/ResonatorBT.h"
#include "../dsp/PitchEnvelope.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/Envelope.h"
#include "../dsp/SVFilter.h"

namespace tr808::voices
{
// Toms (LT/MT/HT) and congas (LC/MC/HC): tuned resonator body + downward pitch
// sweep + attack-noise click, with optional drive. Deep = absolute; macros:
// Tune offsets pitch (+/-12 st), Level trims. setConfig() seeds per-instance
// defaults (so tests differ without an APVTS attached).
class TomCongaVoice : public Voice
{
public:
    void setConfig (float baseFrequency, float decaySeconds) noexcept
    {
        deep.freq      = baseFrequency;
        deep.decayTime = decaySeconds * 1000.0f;
    }

    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

    std::vector<DeepRef> deepRefs() override
    {
        return { { "freq", &deep.freq }, { "penvamt", &deep.penvAmt }, { "penvtime", &deep.penvTime },
                 { "decaytime", &deep.decayTime }, { "atknoise", &deep.atkNoise }, { "drive", &deep.drive } };
    }

private:
    struct Deep
    {
        float freq = 120.0f, penvAmt = 0.6f, penvTime = 40.0f, decayTime = 500.0f,
              atkNoise = 0.3f, drive = 1.0f;
    } deep;

    dsp::ResonatorBT   res;
    dsp::PitchEnvelope pitchEnv;
    dsp::NoiseGen      attackNoise;
    dsp::Envelope      attackEnv;
    dsp::SVFilter      attackHpf;

    float amp         = 0.0f;
    float fBase       = 120.0f;
    float attackLevel = 0.3f;
    float driveAmt    = 1.0f;
};
}
