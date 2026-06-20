#include "CowbellVoice.h"

namespace tr808::voices
{
void CowbellVoice::prepare (double sr, int)
{
    sampleRate = sr;
    o1.prepare (sr); o1.setWaveform (dsp::BandlimitedOsc::Waveform::square); o1.setFrequency (540.0f);
    o2.prepare (sr); o2.setWaveform (dsp::BandlimitedOsc::Waveform::square); o2.setFrequency (800.0f);
    bp.prepare (sr); bp.setType (dsp::SVFilter::Type::bandpass); bp.setCutoff (2640.0f); bp.setResonance (0.8f);
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
    o1.reset(); o2.reset();
    env.setDecay (400.0f);
    env.trigger();
}

void CowbellVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const float s = bp.processSample ((o1.processSample() + o2.processSample()) * 0.5f);
        mono[i] += s * env.processSample() * amp;
    }
}

bool CowbellVoice::isActive() const
{
    return env.isActive();
}
}
