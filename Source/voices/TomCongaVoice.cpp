#include "TomCongaVoice.h"

namespace tr808::voices
{
void TomCongaVoice::prepare (double sr, int)
{
    sampleRate = sr;
    res.prepare (sr);
    pitchEnv.prepare (sr);
    attackNoise.prepare (sr);
    attackEnv.prepare (sr);
    attackEnv.setMode (dsp::Envelope::Mode::ad);
    attackEnv.setAttack (0.2f);
    attackHpf.prepare (sr);
    attackHpf.setType (dsp::SVFilter::Type::highpass);
    attackHpf.setCutoff (2000.0f);
    reset();
}

void TomCongaVoice::reset()
{
    res.reset();
    pitchEnv.reset();
    attackEnv.reset();
    attackHpf.reset();
    amp = 0.0f;
}

void TomCongaVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);

    fBase = baseFreq * mapExp (macros.tune, 0.5f, 2.0f);   // +/- one octave

    res.setFrequency (fBase);
    res.setDecay (decaySec);
    res.reset();
    res.excite (1.0f);

    pitchEnv.setAmount (fBase * 0.6f);
    pitchEnv.setTime (40.0f);
    pitchEnv.trigger();

    attackEnv.setDecay (4.0f);
    attackEnv.trigger();
}

void TomCongaVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        res.setFrequency (fBase + pitchEnv.processSample());
        const float body = res.processSample();
        const float atk  = attackHpf.processSample (attackNoise.processSample())
                         * attackEnv.processSample() * attackLevel;

        mono[i] += (body + atk) * amp;
    }
}

bool TomCongaVoice::isActive() const
{
    return res.isActive() || attackEnv.isActive();
}
}
