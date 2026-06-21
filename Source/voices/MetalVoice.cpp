#include "MetalVoice.h"
#include <algorithm>

namespace tr808::voices
{
void MetalVoice::prepare (double sr, int)
{
    sampleRate = sr;
    cluster.prepare (sr);
    noise.prepare (sr);
    hpf.prepare (sr); hpf.setType (dsp::SVFilter::Type::highpass);
    bp.prepare (sr);  bp.setType (dsp::SVFilter::Type::bandpass); bp.setResonance (0.8f);
    env.prepare (sr); env.setMode (dsp::Envelope::Mode::ad); env.setAttack (0.5f);
    reset();
}

void MetalVoice::reset()
{
    cluster.reset();
    hpf.reset();
    bp.reset();
    env.reset();
    amp = 0.0f;
}

void MetalVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);

    hpf.setCutoff (deep.hpf);
    if (type == Type::cymbal)
    {
        bp.setCutoff (deep.bpFreq);
        toneBal = std::clamp (deep.balance + (macros.tone - 0.5f), 0.0f, 1.0f);   // macro Tone shifts band balance
    }

    // Hats lean on hissy noise; the cymbal stays mostly metallic.
    noiseMix = (type == Type::cymbal) ? 0.12f : 0.62f;

    env.setAttack (deep.attack);
    env.setDecay (deep.decayTime * centeredScale (macros.decay, 3.0f));           // macro Decay scales (neutral for CH)
    cluster.reset();
    env.trigger();
}

void MetalVoice::renderAdd (float* mono, int numSamples)
{
    const bool isCymbal = (type == Type::cymbal);

    for (int i = 0; i < numSamples; ++i)
    {
        const float c   = cluster.processSample();
        const float src = c * (1.0f - noiseMix) + noise.processSample() * noiseMix;
        const float high = hpf.processSample (src);

        float s = high;
        if (isCymbal)
        {
            const float clang = bp.processSample (c);
            s = high * toneBal + clang * (1.0f - toneBal);
        }

        mono[i] += s * env.processSample() * amp;
    }
}

bool MetalVoice::isActive() const
{
    return env.isActive();
}

void MetalVoice::choke()
{
    env.forceRelease (5.0f);
}
}
