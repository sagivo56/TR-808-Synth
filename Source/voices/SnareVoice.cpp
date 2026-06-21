#include "SnareVoice.h"
#include <algorithm>

namespace tr808::voices
{
void SnareVoice::prepare (double sr, int)
{
    sampleRate = sr;
    res1.prepare (sr);
    res2.prepare (sr);
    shellPitch.prepare (sr);
    noise.prepare (sr);
    noiseBp.prepare (sr); noiseBp.setType (dsp::SVFilter::Type::bandpass);
    noiseEnv.prepare (sr); noiseEnv.setMode (dsp::Envelope::Mode::ad); noiseEnv.setAttack (0.5f);
    hpf.prepare (sr); hpf.setType (dsp::SVFilter::Type::highpass);
    reset();
}

void SnareVoice::reset()
{
    res1.reset(); res2.reset();
    shellPitch.reset();
    noiseBp.reset(); noiseEnv.reset();
    hpf.reset();
    amp = 0.0f;
}

void SnareVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);

    o1base = deep.o1freq;
    o2base = deep.o2freq;

    // Two bridged-T resonators excited by the trigger = the membrane body.
    const float decaySec = deep.shellDecay * 0.001f;
    res1.setFrequency (o1base); res1.setDecay (decaySec);        res1.reset(); res1.excite (1.0f);
    res2.setFrequency (o2base); res2.setDecay (decaySec * 0.8f); res2.reset(); res2.excite (1.0f);

    oscMix = std::clamp (deep.oscmix + (macros.tone - 0.5f), 0.0f, 1.0f);   // macro Tone shifts mix

    // Short downward pitch blip = the struck-skin "thwack".
    shellPitch.setAmount (o1base * 0.7f);
    shellPitch.setTime (28.0f);
    shellPitch.trigger();

    noiseBp.setCutoff (deep.nbpFreq);
    noiseBp.setResonance (deep.nbpQ);
    const float snappyMod = centeredScale (macros.snappy, 2.0f);            // macro Snappy scales noise
    shellGain = 1.0f - deep.balance;
    noiseGain = deep.balance * snappyMod;
    noiseEnv.setAttack (deep.attack);
    noiseEnv.setDecay (deep.nDecay * centeredScale (macros.snappy, 2.5f));
    noiseEnv.trigger();

    hpf.setCutoff (deep.hpf);
}

void SnareVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const float pe = shellPitch.processSample();
        res1.setFrequency (o1base + pe);
        res2.setFrequency (o2base + pe * (o2base / o1base));

        const float shell = ((1.0f - oscMix) * res1.processSample()
                           +          oscMix  * res2.processSample()) * 1.5f;
        const float snap  = noiseBp.processSample (noise.processSample())
                          * noiseEnv.processSample();

        mono[i] += hpf.processSample (shell * shellGain + snap * noiseGain) * amp;
    }
}

bool SnareVoice::isActive() const
{
    return res1.isActive() || res2.isActive() || noiseEnv.isActive();
}
}
