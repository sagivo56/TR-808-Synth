#include "ClapVoice.h"
#include <algorithm>
#include <cmath>

namespace tr808::voices
{
void ClapVoice::prepare (double sr, int)
{
    sampleRate = sr;
    noise.prepare (sr);
    bp.prepare (sr);
    bp.setType (dsp::SVFilter::Type::bandpass);
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
    numPulses = std::clamp ((int) std::lround (deep.nPulses), 1, maxPulses);
    const int spacingSamples = std::max (1, (int) (deep.spacing * 0.001 * sampleRate));
    for (int k = 0; k < maxPulses; ++k)
        pulseOffsets[(size_t) k] = k * spacingSamples;

    bp.setCutoff (deep.bpFreq);
    bp.setResonance (deep.bpQ);
    pulseEnv.setDecay (5.0f);
    tailEnv.setDecay (deep.tailDecay);
    pulseEnv.reset();
    tailEnv.reset();
}

void ClapVoice::renderAdd (float* mono, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        if (pulseIndex < numPulses && counter == pulseOffsets[(size_t) pulseIndex])
        {
            pulseEnv.trigger();
            ++pulseIndex;
            if (pulseIndex == numPulses)
                tailEnv.trigger();          // room tail after the last burst
        }

        const float g = pulseEnv.processSample() + 0.6f * tailEnv.processSample();
        mono[i] += bp.processSample (noise.processSample()) * g * amp;
        ++counter;
    }
}

bool ClapVoice::isActive() const
{
    return pulseIndex < numPulses || pulseEnv.isActive() || tailEnv.isActive();
}
}
