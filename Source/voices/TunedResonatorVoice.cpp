#include "TunedResonatorVoice.h"

namespace tr808::voices
{
void TunedResonatorVoice::prepare (double sr, int)
{
    sampleRate = sr;
    res.prepare (sr);
    reset();
}

void TunedResonatorVoice::reset()
{
    res.reset();
    swingVca.reset();
    amp = 0.0f;
}

void TunedResonatorVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);
    res.setFrequency (deep.freq);
    res.setDecay (deep.decayTime * 0.001f);
    res.reset();
    res.excite (1.0f);
}

void TunedResonatorVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float s = res.processSample();
        if (useSwing)
            s = swingVca.process (s);
        mono[i] += s * amp;
    }
}

bool TunedResonatorVoice::isActive() const
{
    return res.isActive();
}
}
