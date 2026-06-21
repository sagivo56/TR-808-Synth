#include "Mixer.h"

#include <algorithm>
#include <cmath>

namespace tr808
{
void Mixer::prepare (double sampleRate, int maxBlockSize)
{
    // 2x-oversampled stereo tanh for the master drive/limiter.
    masterSat.prepare (sampleRate, maxBlockSize, 2, 1);
    masterSat.reset();
}

void Mixer::reset()
{
    masterSat.reset();
}

bool Mixer::anySolo() const
{
    return std::any_of (soloed.begin(), soloed.end(), [] (bool b) { return b; });
}

Mixer::VoiceGain Mixer::gainsFor (int voice) const
{
    if (! valid (voice))
        return { 0.0f, 0.0f, 0.0f };

    const bool solo = anySolo();
    const float gate = (muted[(size_t) voice] || (solo && ! soloed[(size_t) voice])) ? 0.0f : 1.0f;

    // Equal-power pan: -1 => hard left, 0 => centre (-3 dB), +1 => hard right.
    const float angle = (juce::jlimit (-1.0f, 1.0f, panValue[(size_t) voice]) + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
    return { gate, std::cos (angle), std::sin (angle) };
}

void Mixer::processMaster (juce::AudioBuffer<float>& stereo)
{
    masterSat.setDrive (masterDrive);
    juce::dsp::AudioBlock<float> block (stereo);
    masterSat.processBlock (block);
}
}
