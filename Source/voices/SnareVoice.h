#pragma once

#include "Voice.h"
#include "../dsp/BandlimitedOsc.h"
#include "../dsp/Envelope.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/SVFilter.h"

namespace tr808::voices
{
// SD: two tuned sine partials (shell) crossfaded by Tone, plus band-passed
// noise (the "snappy") with its own envelope. Macros: Tone (shell balance),
// Snappy (noise level/decay), Level.
class SnareVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

private:
    dsp::BandlimitedOsc osc1, osc2;
    dsp::Envelope       shellEnv;
    dsp::NoiseGen       noise;
    dsp::SVFilter       noiseBp;
    dsp::Envelope       noiseEnv;

    float amp         = 0.0f;
    float toneBal     = 0.5f;
    float snappyLevel = 0.0f;
};
}
