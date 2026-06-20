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
    amp = 0.0f;
}

void TunedResonatorVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);
    res.setFrequency (freq);
    res.setDecay (decaySec);
    res.reset();
    res.excite (1.0f);
}

void TunedResonatorVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        mono[i] += res.processSample() * amp;
}

bool TunedResonatorVoice::isActive() const
{
    return res.isActive();
}
}
