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
    clickNoise.prepare (sr);
    clickEnv.prepare (sr);
    clickEnv.setMode (dsp::Envelope::Mode::ad);
    clickEnv.setAttack (0.2f);
    clickHpf.prepare (sr);
    clickHpf.setType (dsp::SVFilter::Type::highpass);
    reset();
}

void BassDrumVoice::reset()
{
    sine.reset();
    pitchEnv.reset();
    ampEnv.reset();
    clickEnv.reset();
    clickHpf.reset();
    amp = 0.0f;
}

void BassDrumVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);

    baseFreq = deep.freq;
    sine.reset();
    sine.setFrequency (baseFreq);

    pitchEnv.setAmount (deep.penvAmt);
    pitchEnv.setTime (deep.penvTime);
    pitchEnv.trigger();

    ampEnv.setDecay (deep.bodyDecay * centeredScale (macros.decay, 4.0f));   // macro Decay scales
    ampEnv.trigger();

    driveAmt   = deep.drive;
    clickLevel = deep.clickLvl * centeredScale (macros.tone, 2.5f);          // macro Tone scales click
    clickEnv.setDecay (mapLin (deep.clickTone, 2.0f, 12.0f));
    clickEnv.trigger();
    clickHpf.setCutoff (mapLin (deep.clickTone, 800.0f, 2500.0f));
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

        const float click = clickHpf.processSample (clickNoise.processSample())
                          * clickEnv.processSample() * clickLevel;

        mono[i] += (body + click) * amp;
    }
}

bool BassDrumVoice::isActive() const
{
    return ampEnv.isActive() || clickEnv.isActive();
}
}
