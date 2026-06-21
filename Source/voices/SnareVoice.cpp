#include "SnareVoice.h"
#include <algorithm>

namespace tr808::voices
{
void SnareVoice::prepare (double sr, int)
{
    sampleRate = sr;
    osc1.prepare (sr); osc1.setWaveform (dsp::BandlimitedOsc::Waveform::sine);
    osc2.prepare (sr); osc2.setWaveform (dsp::BandlimitedOsc::Waveform::sine);
    shellEnv.prepare (sr); shellEnv.setMode (dsp::Envelope::Mode::ad); shellEnv.setAttack (1.0f);
    shellPitch.prepare (sr);
    noise.prepare (sr);
    noiseBp.prepare (sr); noiseBp.setType (dsp::SVFilter::Type::bandpass);
    noiseEnv.prepare (sr); noiseEnv.setMode (dsp::Envelope::Mode::ad); noiseEnv.setAttack (0.5f);
    hpf.prepare (sr); hpf.setType (dsp::SVFilter::Type::highpass);
    reset();
}

void SnareVoice::reset()
{
    osc1.reset(); osc2.reset();
    shellEnv.reset();
    noiseBp.reset(); noiseEnv.reset();
    hpf.reset();
    amp = 0.0f;
}

void SnareVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);

    o1base = deep.o1freq;
    o2base = deep.o2freq;
    osc1.setFrequency (o1base);
    osc2.setFrequency (o2base);
    osc1.reset(); osc2.reset();
    oscMix = std::clamp (deep.oscmix + (macros.tone - 0.5f), 0.0f, 1.0f);    // macro Tone shifts mix
    shellEnv.setAttack (deep.attack);
    shellEnv.setDecay (deep.shellDecay);
    shellEnv.trigger();

    // Short downward pitch blip on the body = the struck-skin "thwack".
    shellPitch.setAmount (o1base * 0.7f);
    shellPitch.setTime (28.0f);
    shellPitch.trigger();

    noiseBp.setCutoff (deep.nbpFreq);
    noiseBp.setResonance (deep.nbpQ);
    const float snappyMod = centeredScale (macros.snappy, 2.0f);            // macro Snappy scales noise
    shellGain = 1.0f - deep.balance;
    noiseGain = deep.balance * snappyMod;
    noiseEnv.setDecay (deep.nDecay * centeredScale (macros.snappy, 2.5f));
    noiseEnv.trigger();

    hpf.setCutoff (deep.hpf);
}

void SnareVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const float pe = shellPitch.processSample();
        osc1.setFrequency (o1base + pe);
        osc2.setFrequency (o2base + pe * (o2base / o1base));

        // Boosted, pitch-swept body = the skin/membrane tone.
        const float shell = ((1.0f - oscMix) * osc1.processSample()
                           +          oscMix  * osc2.processSample()) * shellEnv.processSample() * 1.5f;
        const float snap  = noiseBp.processSample (noise.processSample())
                          * noiseEnv.processSample();

        mono[i] += hpf.processSample (shell * shellGain + snap * noiseGain) * amp;
    }
}

bool SnareVoice::isActive() const
{
    return shellEnv.isActive() || noiseEnv.isActive();
}
}
