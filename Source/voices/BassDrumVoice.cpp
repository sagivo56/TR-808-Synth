#include "BassDrumVoice.h"
#include <cmath>

namespace tr808::voices
{
void BassDrumVoice::prepare (double sr, int)
{
    sampleRate = sr;
    sine.prepare (sr);
    sine.setWaveform (dsp::BandlimitedOsc::Waveform::sine);
    pitchEnv.prepare (sr);
    ampEnv.prepare (sr);
    ampEnv.setMode (dsp::Envelope::Mode::ad);
    ampEnv.setAttack (1.0f);
    reset();
}

void BassDrumVoice::reset()
{
    sine.reset();
    pitchEnv.reset();
    ampEnv.reset();
    amp = 0.0f;
}

void BassDrumVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);

    baseFreq = deep.freq;
    sine.reset();
    sine.setFrequency (baseFreq);

    // Tone scales the pitch sweep -> attack punch.
    pitchEnv.setAmount (deep.penvAmt * centeredScale (macros.tone, 2.0f));
    pitchEnv.setTime (deep.penvTime);
    pitchEnv.trigger();

    ampEnv.setDecay (deep.bodyDecay * centeredScale (macros.decay, 4.0f));
    ampEnv.trigger();

    driveAmt = deep.drive;
}

void BassDrumVoice::renderAdd (float* mono, int numSamples)
{
    const bool driven = driveAmt > 1.01f;

    for (int i = 0; i < numSamples; ++i)
    {
        sine.setFrequency (baseFreq + pitchEnv.processSample());
        float body = sine.processSample() * ampEnv.processSample();
        if (driven)
            body = std::tanh (driveAmt * body);

        mono[i] += body * amp;
    }
}

bool BassDrumVoice::isActive() const
{
    return ampEnv.isActive();
}
}
