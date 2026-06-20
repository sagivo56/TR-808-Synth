#include "BassDrumVoice.h"

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
    clickHpf.setCutoff (1200.0f);
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

    sine.reset();
    sine.setFrequency (baseFreq);

    pitchEnv.setAmount (180.0f);          // Hz of downward sweep
    pitchEnv.setTime (45.0f);
    pitchEnv.trigger();

    ampEnv.setDecay (mapExp (macros.decay, 80.0f, 1200.0f));
    ampEnv.trigger();

    clickLevel = mapLin (macros.tone, 0.05f, 0.6f);
    clickEnv.setDecay (mapLin (macros.tone, 2.0f, 10.0f));
    clickEnv.trigger();
}

void BassDrumVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const float f = baseFreq + pitchEnv.processSample();
        sine.setFrequency (f);

        const float body  = sine.processSample() * ampEnv.processSample();
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
