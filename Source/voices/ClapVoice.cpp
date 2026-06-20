#include "ClapVoice.h"

namespace tr808::voices
{
void ClapVoice::prepare (double sr, int)
{
    sampleRate = sr;
    noise.prepare (sr);
    bp.prepare (sr);
    bp.setType (dsp::SVFilter::Type::bandpass);
    bp.setCutoff (1000.0f);
    bp.setResonance (1.3f);
    pulseEnv.prepare (sr); pulseEnv.setMode (dsp::Envelope::Mode::ad); pulseEnv.setAttack (0.1f);
    tailEnv.prepare (sr);  tailEnv.setMode (dsp::Envelope::Mode::ad);  tailEnv.setAttack (0.1f);
    reset();
}

void ClapVoice::reset()
{
    bp.reset();
    pulseEnv.reset();
    tailEnv.reset();
    amp = 0.0f;
    counter = 0;
    pulseIndex = 0;
}

void ClapVoice::trigger (float velocity, bool accent)
{
    amp = triggerAmp (velocity, accent);

    counter = 0;
    pulseIndex = 0;
    const int spacing = (int) (0.010 * sampleRate);   // ~10 ms between bursts
    pulseOffsets = { { 0, spacing, 2 * spacing } };

    pulseEnv.setDecay (5.0f);
    tailEnv.setDecay (120.0f);
    pulseEnv.reset();
    tailEnv.reset();
}

void ClapVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        if (pulseIndex < 3 && counter == pulseOffsets[(size_t) pulseIndex])
        {
            pulseEnv.trigger();
            ++pulseIndex;
            if (pulseIndex == 3)
                tailEnv.trigger();      // room tail after the last burst
        }

        const float g = pulseEnv.processSample() + 0.6f * tailEnv.processSample();
        mono[i] += bp.processSample (noise.processSample()) * g * amp;
        ++counter;
    }
}

bool ClapVoice::isActive() const
{
    return pulseIndex < 3 || pulseEnv.isActive() || tailEnv.isActive();
}
}
