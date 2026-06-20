#include "MaracasVoice.h"

namespace tr808::voices
{
void MaracasVoice::prepare (double sr, int)
{
    sampleRate = sr;
    noise.prepare (sr);
    hpf.prepare (sr);
    hpf.setType (dsp::SVFilter::Type::highpass);
    hpf.setCutoff (6000.0f);
    env.prepare (sr);
    env.setMode (dsp::Envelope::Mode::ad);
    env.setAttack (0.5f);
    reset();
}

void MaracasVoice::reset()
{
    hpf.reset();
    env.reset();
    amp = 0.0f;
}

void MaracasVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);
    env.setDecay (30.0f);
    env.trigger();
}

void MaracasVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        mono[i] += hpf.processSample (noise.processSample()) * env.processSample() * amp;
}

bool MaracasVoice::isActive() const
{
    return env.isActive();
}
}
