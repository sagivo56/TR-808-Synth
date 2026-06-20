#include "MetalVoice.h"

namespace tr808::voices
{
void MetalVoice::prepare (double sr, int)
{
    sampleRate = sr;
    cluster.prepare (sr);
    hpf.prepare (sr);
    hpf.setType (dsp::SVFilter::Type::highpass);
    hpf.setCutoff (type == Type::cymbal ? 5000.0f : 7000.0f);
    bp.prepare (sr);
    bp.setType (dsp::SVFilter::Type::bandpass);
    bp.setCutoff (3200.0f);
    bp.setResonance (0.8f);
    env.prepare (sr);
    env.setMode (dsp::Envelope::Mode::ad);
    env.setAttack (0.5f);
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
    amp     = triggerAmp (velocity, accent);
    toneBal = macros.tone;

    switch (type)
    {
        case Type::closedHat: env.setDecay (50.0f); break;
        case Type::openHat:   env.setDecay (mapExp (macros.decay, 150.0f, 1500.0f)); break;
        case Type::cymbal:    env.setDecay (mapExp (macros.decay, 400.0f, 4000.0f)); break;
    }

    cluster.reset();
    env.trigger();
}

void MetalVoice::renderAdd (float* mono, int numSamples)
{
    const bool isCymbal = (type == Type::cymbal);

    for (int i = 0; i < numSamples; ++i)
    {
        const float c    = cluster.processSample();
        const float high = hpf.processSample (c);

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
    env.forceRelease (5.0f);    // fast fade to avoid a click
}
}
