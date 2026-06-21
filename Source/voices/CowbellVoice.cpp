#include "CowbellVoice.h"

namespace tr808::voices
{
void CowbellVoice::prepare (double sr, int)
{
    sampleRate = sr;
    o1.prepare (sr);
    o2.prepare (sr);
    bp.prepare (sr); bp.setType (dsp::SVFilter::Type::bandpass);
    env.prepare (sr); env.setMode (dsp::Envelope::Mode::ad); env.setAttack (1.0f);
    reset();
}

void CowbellVoice::reset()
{
    o1.reset(); o2.reset();
    bp.reset();
    env.reset();
    amp = 0.0f;
}

void CowbellVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);
    o1.setFrequency (deep.o1freq);
    o2.setFrequency (deep.o2freq);
    o1.reset(); o2.reset();
    mix = deep.oscmix;
    bp.setCutoff (deep.bpFreq);
    bp.setResonance (deep.bpQ);
    env.setAttack (deep.attack);
    env.setDecay (deep.decayTime);
    env.trigger();
}

void CowbellVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const float oscs = (1.0f - mix) * o1.processSample() + mix * o2.processSample();
        mono[i] += bp.processSample (oscs) * env.processSample() * amp;
    }
}

bool CowbellVoice::isActive() const
{
    return env.isActive();
}
}
