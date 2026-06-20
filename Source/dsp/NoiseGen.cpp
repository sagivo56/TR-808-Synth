#include "NoiseGen.h"
#include <cmath>

namespace tr808::dsp
{
static constexpr double kTwoPi = 6.283185307179586476925286766559;

static inline float onePoleCoef (float hz, double sr) noexcept
{
    return (float) std::exp (-kTwoPi * (double) hz / sr);
}

void NoiseGen::prepare (double sr) { sampleRate = sr; reset(); }
void NoiseGen::reset()             { lpState = hpState = 0.0f; }

void NoiseGen::setLowpass (float hz) noexcept
{
    lpOn = hz > 0.0f;
    if (lpOn) lpCoef = onePoleCoef (hz, sampleRate);
}

void NoiseGen::setHighpass (float hz) noexcept
{
    hpOn = hz > 0.0f;
    if (hpOn) hpCoef = onePoleCoef (hz, sampleRate);
}

float NoiseGen::white() noexcept
{
    // xorshift32 -> [-1, 1)
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return (float) ((int32_t) rng) * (1.0f / 2147483648.0f);
}

float NoiseGen::processSample() noexcept
{
    float x = white();

    if (lpOn)
    {
        lpState = lpState * lpCoef + x * (1.0f - lpCoef);   // one-pole low-pass
        x = lpState;
    }
    if (hpOn)
    {
        hpState = hpState * hpCoef + x * (1.0f - hpCoef);   // low-pass...
        x = x - hpState;                                     // ...subtracted = high-pass
    }
    return x;
}
}
