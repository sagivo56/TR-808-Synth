#pragma once

#include <cstdint>

namespace tr808::dsp
{
//==============================================================================
// White noise (fast xorshift) with optional one-pole low-pass and high-pass
// shaping to produce the "colored" noise used by snare/clap/hats/maracas.
// Allocation-free and deterministic given a seed.
//==============================================================================
class NoiseGen
{
public:
    void prepare (double sampleRate);
    void reset();

    void setLowpass  (float hz) noexcept;     // <= 0 disables
    void setHighpass (float hz) noexcept;     // <= 0 disables
    void setSeed (uint32_t s) noexcept { rng = (s != 0 ? s : 1u); }

    float processSample() noexcept;

private:
    float white() noexcept;

    double   sampleRate = 44100.0;
    uint32_t rng = 0x1234567u;

    bool  lpOn = false, hpOn = false;
    float lpCoef = 0.0f, hpCoef = 0.0f;
    float lpState = 0.0f, hpState = 0.0f;
};
}
