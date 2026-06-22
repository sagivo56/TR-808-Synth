#pragma once

#include "Voice.h"
#include "../dsp/MetalCluster.h"
#include "../dsp/SVFilter.h"
#include "../dsp/Envelope.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/SwingVCA.h"

namespace tr808::voices
{
// CY / OH / CH: the inharmonic MetalCluster shaped per voice. Deep: HPF + Decay
// (all), plus Clang band + Band balance for the cymbal. Macros: CY Tone shifts
// the band balance, CY/OH Decay scales decay, Level trims. Hats choke fast.
class MetalVoice : public Voice
{
public:
    enum class Type { cymbal, openHat, closedHat };

    void setType (Type t) noexcept { type = t; }

    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;
    void choke() override;

    std::vector<DeepRef> deepRefs() override
    {
        if (type == Type::cymbal)
            return { { "hpf", &deep.hpf }, { "bpfreq", &deep.bpFreq },
                     { "decaytime", &deep.decayTime }, { "balance", &deep.balance }, { "attack", &deep.attack } };
        return { { "hpf", &deep.hpf }, { "lpf", &deep.lpFreq },
                 { "decaytime", &deep.decayTime }, { "attack", &deep.attack } };
    }

private:
    struct Deep
    {
        float hpf = 7000.0f, bpFreq = 3200.0f, decayTime = 500.0f, balance = 0.5f, attack = 0.5f;
        float lpFreq = 11000.0f;  // hat top roll-off (Color): real 808 hats band-limit ~11 kHz
    } deep;

    dsp::MetalCluster cluster;
    dsp::NoiseGen     noise;
    dsp::SwingVCA     swingVca;    // cymbal diode harmonics (Fig 12)
    dsp::SVFilter     hpf;
    dsp::SVFilter     lpf;         // hat top roll-off (Color)
    dsp::SVFilter     bp;
    dsp::Envelope     env;

    Type  type     = Type::closedHat;
    float amp      = 0.0f;
    float toneBal  = 0.5f;
    float noiseMix = 0.0f;   // authentic hats use 0 (pure metal cluster); cymbal a hair
};
}
