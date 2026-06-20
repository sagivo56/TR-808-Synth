#include "MetalCluster.h"

namespace tr808::dsp
{
void MetalCluster::prepare (double sr)
{
    for (int i = 0; i < numOscillators; ++i)
    {
        oscs[(size_t) i].prepare (sr);
        oscs[(size_t) i].setWaveform (BandlimitedOsc::Waveform::square);
        oscs[(size_t) i].setFrequency (freqs[(size_t) i]);
    }
}

void MetalCluster::reset()
{
    for (auto& o : oscs)
        o.reset();
}

void MetalCluster::setFrequency (int index, float hz) noexcept
{
    if (index >= 0 && index < numOscillators)
    {
        freqs[(size_t) index] = hz;
        oscs[(size_t) index].setFrequency (hz);
    }
}

void MetalCluster::setFrequencies (const std::array<float, numOscillators>& f) noexcept
{
    for (int i = 0; i < numOscillators; ++i)
    {
        freqs[(size_t) i] = f[(size_t) i];
        oscs[(size_t) i].setFrequency (f[(size_t) i]);
    }
}

float MetalCluster::processSample() noexcept
{
    float sum = 0.0f;
    for (auto& o : oscs)
        sum += o.processSample();

    return sum * (1.0f / (float) numOscillators);
}
}
