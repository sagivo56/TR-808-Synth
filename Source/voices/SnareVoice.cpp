#include "SnareVoice.h"

namespace tr808::voices
{
void SnareVoice::prepare (double sr, int)
{
    sampleRate = sr;
    osc1.prepare (sr); osc1.setWaveform (dsp::BandlimitedOsc::Waveform::sine); osc1.setFrequency (180.0f);
    osc2.prepare (sr); osc2.setWaveform (dsp::BandlimitedOsc::Waveform::sine); osc2.setFrequency (330.0f);
    shellEnv.prepare (sr); shellEnv.setMode (dsp::Envelope::Mode::ad); shellEnv.setAttack (1.0f);
    noise.prepare (sr);
    noiseBp.prepare (sr); noiseBp.setType (dsp::SVFilter::Type::bandpass); noiseBp.setCutoff (1800.0f); noiseBp.setResonance (1.1f);
    noiseEnv.prepare (sr); noiseEnv.setMode (dsp::Envelope::Mode::ad); noiseEnv.setAttack (0.5f);
    reset();
}

void SnareVoice::reset()
{
    osc1.reset(); osc2.reset();
    shellEnv.reset();
    noiseBp.reset(); noiseEnv.reset();
    amp = 0.0f;
}

void SnareVoice::trigger (float velocity, bool accent)
{
    amp     = triggerAmp (velocity, accent);
    toneBal = macros.tone;

    osc1.reset(); osc2.reset();
    shellEnv.setDecay (130.0f);
    shellEnv.trigger();

    snappyLevel = mapLin (macros.snappy, 0.2f, 1.2f);
    noiseEnv.setDecay (mapExp (macros.snappy, 80.0f, 400.0f));
    noiseEnv.trigger();
}

void SnareVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const float shell = ((1.0f - toneBal) * osc1.processSample()
                           +         toneBal  * osc2.processSample()) * shellEnv.processSample();

        const float snap  = noiseBp.processSample (noise.processSample())
                          * noiseEnv.processSample() * snappyLevel;

        mono[i] += (shell * 0.8f + snap) * amp;
    }
}

bool SnareVoice::isActive() const
{
    return shellEnv.isActive() || noiseEnv.isActive();
}
}
