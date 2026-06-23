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
    clickEnv = 0.0f;
}

void BassDrumVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);
    // Pitch: a melodic bass note (playFreq) wins; otherwise the deep "Pitch" base
    // shifted +/- 1 octave by the Tune macro (neutral at 0.5).
    baseFreq = (playFreq > 0.0f) ? playFreq : deep.freq * centeredPitch (macros.tune, 12.0f);

    // DECAY: Q/feedback -> ring (decay) time; macro Decay scales it, Sustain
    // extends it dramatically (up to ~9x) for a long booming tail.
    res.setDecay (deep.decay * 0.001f * centeredScale (macros.decay, 4.0f) * (1.0f + deep.sustain * 8.0f));

    // First half-cycle at the inherent pitch overshoot (deep Punch). This is the
    // circuit's own pitch envelope, not a front-panel control.
    const float startMult = std::clamp (deep.punch, 1.0f, 4.0f);
    const float startFreq = baseFreq * startMult;
    res.setFrequency (startFreq);
    res.reset();
    res.excite (1.0f);

    switchSample = (int) (0.5 / (double) startFreq * sampleRate);   // half a cycle at startFreq
    sampleCount  = 0;
    switched     = false;
    retrigAmt    = deep.retrig;
    driveAmt     = deep.drive;

    // TONE: arm the beater click. A short bright burst (~2 kHz, ~1.2 ms) summed
    // into the output; the Tone macro is its level (0 => pure sine body).
    clickAmt   = macros.tone;
    clickEnv   = 1.0f;
    clickPhase = 0.0f;
    clickInc   = (float) (6.283185307179586 * 2000.0 / sampleRate);
    clickDecay = std::exp (-1.0f / (0.0012f * (float) sampleRate));
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

        // TONE: feed-forward beater click (bright, very fast decay).
        if (clickEnv > 1.0e-4f)
        {
            s += std::sin (clickPhase) * clickEnv * clickAmt * 0.5f;
            clickPhase += clickInc;
            clickEnv   *= clickDecay;
        }

        if (driven)
            s = std::tanh (driveAmt * s);

        mono[i] += s * amp;
        ++sampleCount;
    }
}

bool BassDrumVoice::isActive() const
{
    return res.isActive() || ! switched || clickEnv > 1.0e-4f;
}
}
