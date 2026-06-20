#include "Saturator.h"
#include <cmath>

namespace tr808::dsp
{
float Saturator::shape (float x, float drive) noexcept
{
    return std::tanh (drive * x);
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
