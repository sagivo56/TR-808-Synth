#pragma once

#include "Voice.h"
#include "../dsp/ResonatorBT.h"
#include "../dsp/PitchEnvelope.h"
#include "../dsp/Envelope.h"
#include "../dsp/NoiseGen.h"
#include "../dsp/SVFilter.h"

namespace tr808::voices
{
// SD — service-manual circuit model: two bridged-T resonators (decaying-sine
// "skin"/shell tones, the fundamental + harmonic networks) mixed by a balance,
// plus band-passed noise (the "snappy"), high-passed. A short downward pitch
// blip gives the struck-skin attack. Macros: Tone shifts the resonator mix,
// Snappy scales the noise, Level.
class SnareVoice : public Voice
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
                 { "shelldecay", &deep.shellDecay }, { "nbpfreq", &deep.nbpFreq }, { "nbpq", &deep.nbpQ },
                 { "ndecay", &deep.nDecay }, { "hpf", &deep.hpf }, { "balance", &deep.balance },
                 { "attack", &deep.attack } };
    }

private:
    struct Deep
    {
        float o1freq = 180.0f, o2freq = 330.0f, oscmix = 0.5f, shellDecay = 130.0f,
              nbpFreq = 1800.0f, nbpQ = 1.1f, nDecay = 200.0f, hpf = 300.0f, balance = 0.5f,
              attack = 1.0f;
    } deep;

    dsp::ResonatorBT    res1, res2;     // bridged-T fundamental + harmonic networks
    dsp::PitchEnvelope  shellPitch;     // membrane "skin" attack
    dsp::NoiseGen       noise;
    dsp::SVFilter       noiseBp;
    dsp::Envelope       noiseEnv;
    dsp::SVFilter       hpf;

    float amp       = 0.0f;
    float oscMix    = 0.5f;
    float shellGain = 0.5f;
    float noiseGain = 0.5f;
    float o1base    = 180.0f;
    float o2base    = 330.0f;
};
}
