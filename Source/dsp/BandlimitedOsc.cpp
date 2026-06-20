#include "BandlimitedOsc.h"
#include <cmath>

namespace tr808::dsp
{
static constexpr double kTwoPi = 6.283185307179586476925286766559;

void BandlimitedOsc::prepare (double sr)
{
    sampleRate = sr;
    setFrequency (frequency);
    reset();
}

void BandlimitedOsc::reset()
{
    phase = 0.0;
    triState = 0.0f;
}

void BandlimitedOsc::setFrequency (float hz) noexcept
{
    frequency = hz;
    phaseInc  = (double) hz / sampleRate;
}

float BandlimitedOsc::polyBlep (double t, double dt) noexcept
{
    // Polynomial correction around a unit step at t = 0 / t = 1.
    if (dt <= 0.0)
        return 0.0f;

    if (t < dt)                 { t /= dt;             return (float) (t + t - t * t - 1.0); }
    if (t > 1.0 - dt)           { t = (t - 1.0) / dt;  return (float) (t * t + t + t + 1.0); }
    return 0.0f;
}

float BandlimitedOsc::processSample() noexcept
{
    const double dt = phaseInc;
    float out = 0.0f;

    switch (waveform)
    {
        case Waveform::sine:
            out = (float) std::sin (kTwoPi * phase);
            break;

        case Waveform::square:
        {
            double v = (phase < 0.5 ? 1.0 : -1.0);
            v += polyBlep (phase, dt);                       // rising edge at phase 0
            double t2 = phase + 0.5; if (t2 >= 1.0) t2 -= 1.0;
            v -= polyBlep (t2, dt);                           // falling edge at phase 0.5
            out = (float) v;
            break;
        }

        case Waveform::triangle:
        {
            double sq = (phase < 0.5 ? 1.0 : -1.0);
            sq += polyBlep (phase, dt);
            double t2 = phase + 0.5; if (t2 >= 1.0) t2 -= 1.0;
            sq -= polyBlep (t2, dt);
            // Integrate the band-limited square; the small leak removes DC drift.
            triState = (float) (triState * 0.9995 + 4.0 * dt * sq);
            out = triState;
            break;
        }
    }

    phase += dt;
    if (phase >= 1.0)
        phase -= 1.0;

    return out;
}
}
