#pragma once

#include "Voice.h"
#include "../dsp/ResonatorBT.h"

namespace tr808::voices
{
// RS (rim shot) and CL (claves): a short, high-pitched impulse-excited
// resonator. Macro: Level. (Tune/Decay/Tone become Deep-edit params in M3.)
class TunedResonatorVoice : public Voice
{
public:
    void setConfig (float frequency, float decaySeconds) noexcept
    {
        freq     = frequency;
        decaySec = decaySeconds;
    }

    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

private:
    dsp::ResonatorBT res;
    float amp      = 0.0f;
    float freq     = 1700.0f;
    float decaySec = 0.06f;
};
}
