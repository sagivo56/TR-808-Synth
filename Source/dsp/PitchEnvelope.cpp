#include "PitchEnvelope.h"
#include <cmath>
#include <algorithm>

namespace tr808::dsp
{
void PitchEnvelope::prepare (double sr)
{
    sampleRate = sr;
    setTime (timeMs);
    reset();
}

void PitchEnvelope::reset()
{
    value = 0.0f;
    active = false;
}

void PitchEnvelope::setTime (float ms) noexcept
{
    timeMs = std::max (1.0f, ms);
    const double samples = std::max (1.0, (double) timeMs * 0.001 * sampleRate);
    // Decay to 0.1% of 'amount' over 'time'.
    decayMul = (float) std::exp (std::log (0.001) / samples);
}

float PitchEnvelope::processSample() noexcept
{
    if (! active)
        return 0.0f;

    const float current = value;
    value *= decayMul;

    if (std::abs (value) < 0.001f * (std::abs (amountValue) + 1.0e-9f))
    {
        value = 0.0f;
        active = false;
    }
    return current;
}
}
