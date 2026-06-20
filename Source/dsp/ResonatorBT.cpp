#include "ResonatorBT.h"
#include <cmath>
#include <algorithm>

namespace tr808::dsp
{
static constexpr double kTwoPi = 6.283185307179586476925286766559;

void ResonatorBT::prepare (double sr)
{
    sampleRate = sr;
    update();
    reset();
}

void ResonatorBT::reset()
{
    y1 = y2 = pending = 0.0f;
}

void ResonatorBT::setFrequency (float hz) noexcept
{
    frequency = std::clamp (hz, 1.0f, (float) (sampleRate * 0.49));
    update();
}

void ResonatorBT::setDecay (float seconds) noexcept
{
    decaySeconds = std::max (0.001f, seconds);
    update();
}

void ResonatorBT::update() noexcept
{
    const double w = kTwoPi * (double) frequency / sampleRate;
    // Pole radius giving -60 dB after 'decaySeconds'.
    const double r = std::exp (std::log (0.001) / (double) (decaySeconds * sampleRate));
    a1 = (float) (2.0 * r * std::cos (w));
    a2 = (float) (-(r * r));
    gainComp = (float) std::sin (w);
}

float ResonatorBT::processSample() noexcept
{
    const float x = pending;
    pending = 0.0f;

    const float y = x + a1 * y1 + a2 * y2;
    y2 = y1;
    y1 = y;
    return y;
}

bool ResonatorBT::isActive() const noexcept
{
    return (std::abs (y1) + std::abs (y2) + std::abs (pending)) > 1.0e-5f;
}
}
