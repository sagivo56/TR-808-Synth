#include "SVFilter.h"
#include <cmath>
#include <algorithm>

namespace tr808::dsp
{
static constexpr double kPi = 3.14159265358979323846;

void SVFilter::prepare (double sr)
{
    sampleRate = sr;
    updateCoefficients();
    reset();
}

void SVFilter::reset()
{
    ic1eq = ic2eq = 0.0f;
}

void SVFilter::setCutoff (float hz) noexcept
{
    const float nyq = (float) (sampleRate * 0.49);
    cutoff = std::clamp (hz, 20.0f, nyq);
    updateCoefficients();
}

void SVFilter::setResonance (float newQ) noexcept
{
    q = std::clamp (newQ, 0.05f, 40.0f);
    updateCoefficients();
}

void SVFilter::updateCoefficients() noexcept
{
    g  = (float) std::tan (kPi * (double) cutoff / sampleRate);
    k  = 1.0f / q;
    a1 = 1.0f / (1.0f + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
}

float SVFilter::processSample (float x) noexcept
{
    const float v3 = x - ic2eq;
    const float v1 = a1 * ic1eq + a2 * v3;
    const float v2 = ic2eq + a2 * ic1eq + a3 * v3;

    ic1eq = 2.0f * v1 - ic1eq;
    ic2eq = 2.0f * v2 - ic2eq;

    switch (type)
    {
        case Type::lowpass:  return v2;
        case Type::highpass: return x - k * v1 - v2;
        case Type::bandpass: return v1;
    }
    return v2;
}
}
