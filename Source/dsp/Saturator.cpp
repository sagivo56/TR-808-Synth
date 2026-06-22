#include "Saturator.h"
#include <cmath>

namespace tr808::dsp
{
float Saturator::shape (float x, float drive) noexcept
{
    // Soft limiter with a linear "knee": transparent below the knee so a clean
    // mix at unity drive passes through uncoloured, and only peaks approaching
    // full scale are soft-clipped (asymptotes to 1.0, never clips hard). Turning
    // drive up pushes more signal past the knee, restoring the saturation/drive.
    const float y = drive * x;
    const float a = std::abs (y);
    constexpr float knee = 0.7f;
    if (a <= knee)
        return y;
    const float over = (a - knee) / (1.0f - knee);
    return std::copysign (knee + (1.0f - knee) * std::tanh (over), y);
}

void Saturator::prepare (double /*sampleRate*/, int maxBlockSize, int numChannels, int oversampleLog2)
{
    oversampling = std::make_unique<juce::dsp::Oversampling<float>> (
        (size_t) juce::jmax (1, numChannels),
        (size_t) juce::jmax (0, oversampleLog2),
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);

    oversampling->initProcessing ((size_t) juce::jmax (1, maxBlockSize));
    oversampling->reset();
}

void Saturator::reset()
{
    if (oversampling != nullptr)
        oversampling->reset();
}

void Saturator::processBlock (juce::dsp::AudioBlock<float> block)
{
    if (oversampling == nullptr)
    {
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* d = block.getChannelPointer (ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
                d[i] = shape (d[i], drive);
        }
        return;
    }

    auto up = oversampling->processSamplesUp (block);
    for (size_t ch = 0; ch < up.getNumChannels(); ++ch)
    {
        auto* d = up.getChannelPointer (ch);
        for (size_t i = 0; i < up.getNumSamples(); ++i)
            d[i] = shape (d[i], drive);
    }
    oversampling->processSamplesDown (block);
}

int Saturator::getLatencySamples() const noexcept
{
    return oversampling != nullptr ? (int) oversampling->getLatencyInSamples() : 0;
}
}
