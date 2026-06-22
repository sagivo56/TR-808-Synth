#include "BassDrumVoice.h"
#include <cmath>
#include <algorithm>

namespace tr808::voices
{
void BassDrumVoice::prepare (double sr, int)
{
    sampleRate = sr;
    res.prepare (sr);
    reset();
}

void BassDrumVoice::reset()
{
    res.reset();
    amp = 0.0f;
    sampleCount = 0;
    switched = true;
}

void BassDrumVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);
    baseFreq = deep.freq;

    // Q/feedback -> ring (decay) time; macro Decay scales it, Sustain extends it
    // dramatically (up to ~9x) for a long booming tail.
    res.setDecay (deep.decay * 0.001f * centeredScale (macros.decay, 4.0f) * (1.0f + deep.sustain * 8.0f));

    // First half-cycle at the "punch" multiple of the inherent frequency.
    const float startMult = std::clamp (deep.punch * centeredScale (macros.tone, 1.5f), 1.0f, 4.0f);
    const float startFreq = baseFreq * startMult;
    res.setFrequency (startFreq);
    res.reset();
    res.excite (1.0f);

    switchSample = (int) (0.5 / (double) startFreq * sampleRate);   // half a cycle at startFreq
    sampleCount  = 0;
    switched     = false;
    retrigAmt    = deep.retrig;
    driveAmt     = deep.drive;
}

void BassDrumVoice::renderAdd (float* mono, int numSamples)
{
    const bool driven = driveAmt > 1.01f;

    for (int i = 0; i < numSamples; ++i)
    {
        if (! switched && sampleCount >= switchSample)
        {
            res.setFrequency (baseFreq);       // drop to the inherent frequency
            res.excite (retrigAmt);            // C39/R161 retrigger pulse
            switched = true;
        }

        float s = res.processSample();
        if (driven)
            s = std::tanh (driveAmt * s);

        mono[i] += s * amp;
        ++sampleCount;
    }
}

bool BassDrumVoice::isActive() const
{
    return res.isActive() || ! switched;
}
}
