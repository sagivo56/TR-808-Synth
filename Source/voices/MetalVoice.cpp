#include "MetalVoice.h"
#include <algorithm>

namespace tr808::voices
{
void MetalVoice::prepare (double sr, int)
{
    sampleRate = sr;
    cluster.prepare (sr);
    noise.prepare (sr);
    hpf.prepare (sr); hpf.setType (dsp::SVFilter::Type::highpass);
    lpf.prepare (sr); lpf.setType (dsp::SVFilter::Type::lowpass);
    bp.prepare (sr);  bp.setType (dsp::SVFilter::Type::bandpass); bp.setResonance (0.8f);
    env.prepare (sr); env.setMode (dsp::Envelope::Mode::ad); env.setAttack (0.5f);
    reset();
}

void MetalVoice::reset()
{
    cluster.reset();
    swingVca.reset();
    hpf.reset();
    lpf.reset();
    bp.reset();
    env.reset();
    amp = 0.0f;
}

void MetalVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);

    hpf.setCutoff (deep.hpf);
    if (type == Type::cymbal)
    {
        bp.setCutoff (deep.bpFreq);
        toneBal = std::clamp (deep.balance + (macros.tone - 0.5f), 0.0f, 1.0f);   // macro Tone shifts band balance
    }
    else
    {
        hpf.setResonance (1.3f);   // a little ring/emphasis for the metallic hat top
        lpf.setCutoff (deep.lpFreq);   // Color: top roll-off so the hat isn't thin/harsh
    }

    // Authentic 808 hats are the pure 6-oscillator metallic cluster (no noise);
    // CH/OH differ only in decay. The cymbal keeps a hair of air.
    noiseMix = (type == Type::cymbal) ? 0.12f : 0.0f;

    env.setAttack (deep.attack);
    env.setDecay (deep.decayTime * centeredScale (macros.decay, 3.0f));           // macro Decay scales (neutral for CH)
    cluster.reset();
    env.trigger();
}

void MetalVoice::renderAdd (float* mono, int numSamples)
{
    const bool isCymbal = (type == Type::cymbal);

    for (int i = 0; i < numSamples; ++i)
    {
        float c = cluster.processSample();
        if (isCymbal)
            c = swingVca.process (c);            // diode -> ringing metallic harmonics
        const float src = c * (1.0f - noiseMix) + noise.processSample() * noiseMix;
        const float high = hpf.processSample (src);

        float s = high;
        if (isCymbal)
        {
            const float clang = bp.processSample (c);
            s = high * toneBal + clang * (1.0f - toneBal);
        }
        else
        {
            s = lpf.processSample (high);   // Color: band-limit the hat top (real 808 rolls off)
        }

        mono[i] += s * env.processSample() * amp;
    }
}

bool MetalVoice::isActive() const
{
    return env.isActive();
}

void MetalVoice::choke()
{
    env.forceRelease (5.0f);
}
}
