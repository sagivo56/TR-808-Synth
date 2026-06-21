#include "SchmittOsc.h"
#include <cmath>

namespace tr808::dsp
{
void SchmittOsc::prepare (double sr)
{
    sampleRate = sr;
    slewCoef = 0.5f;     // output edge rate (lower = softer/darker, fewer alias)
    reset();
}

void SchmittOsc::reset()
{
    vcap = 0.0f;
    out = 1.0f;
    outSlew = 0.0f;
}

void SchmittOsc::setFrequency (float hz) noexcept
{
    // For an RC relaxation oscillator with symmetric thresholds the period is
    //   T = R*C * [ ln((1-thLo)/(1-thHi)) + ln((1+thHi)/(1+thLo)) ].
    // We discretise the one-pole charge as v += (out - v) * rate with
    //   rate = dt / (R*C),  dt = 1 / (oversample * sr),  R*C = 1 / (hz * k).
    const double k  = std::log ((1.0 - thLo) / (1.0 - thHi)) + std::log ((1.0 + thHi) / (1.0 + thLo));
    const double dt = 1.0 / (kOversample * sampleRate);
    rate = (float) (dt * (double) hz * k);
}

float SchmittOsc::processSample() noexcept
{
    float acc = 0.0f;

    for (int i = 0; i < kOversample; ++i)
    {
        vcap += (out - vcap) * rate;                       // RC charge toward the output rail

        if (out > 0.0f && vcap >= thHi)      out = -1.0f;  // Schmitt flip
        else if (out < 0.0f && vcap <= thLo) out =  1.0f;

        outSlew += (out - outSlew) * slewCoef;             // finite output edge
        acc += outSlew;
    }

    return acc * (1.0f / (float) kOversample);             // box-average decimation
}
}
