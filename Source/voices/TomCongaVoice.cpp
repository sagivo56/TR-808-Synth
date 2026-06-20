#include "TomCongaVoice.h"
#include <cmath>

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

    fBase = deep.freq * centeredPitch (macros.tune, 12.0f);     // macro Tune: +/- one octave

    res.setFrequency (fBase);
    res.setDecay (deep.decayTime * 0.001f);
    res.reset();
    res.excite (1.0f);

    pitchEnv.setAmount (fBase * deep.penvAmt);
    pitchEnv.setTime (deep.penvTime);
    pitchEnv.trigger();

    attackLevel = deep.atkNoise;
    attackEnv.setDecay (4.0f);
    attackEnv.trigger();

    driveAmt = deep.drive;
}

void TomCongaVoice::renderAdd (float* mono, int numSamples)
{
    const bool driven = driveAmt > 1.01f;

    for (int i = 0; i < numSamples; ++i)
    {
        res.setFrequency (fBase + pitchEnv.processSample());
        float body = res.processSample();
        if (driven)
            body = std::tanh (driveAmt * body);

        const float atk = attackHpf.processSample (attackNoise.processSample())
                        * attackEnv.processSample() * attackLevel;

        mono[i] += (body + atk) * amp;
    }
}

bool TomCongaVoice::isActive() const
{
    return res.isActive() || attackEnv.isActive();
}
}
