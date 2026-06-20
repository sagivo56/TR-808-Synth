#pragma once

namespace tr808::dsp
{
//==============================================================================
// Tuned two-pole resonator, the clean stand-in for the 808's bridged-T network.
// excite() injects an impulse; the resonator then rings as a decaying sinusoid
// at the set frequency. This gives the body/ring of BD, toms and congas without
// any samples.
//==============================================================================
class ResonatorBT
{
public:
    void prepare (double sampleRate);
    void reset();

    void setFrequency (float hz) noexcept;
    void setDecay (float seconds) noexcept;          // time to -60 dB

    // gainComp normalises the impulse-response peak (~1/sin(w)) so the ring is
    // ~unity amplitude regardless of frequency.
    void excite (float amplitude) noexcept { pending += amplitude * gainComp; }

    float processSample() noexcept;
    bool  isActive() const noexcept;

private:
    void update() noexcept;

    double sampleRate   = 44100.0;
    float  frequency    = 100.0f;
    float  decaySeconds = 0.3f;

    float a1 = 0.0f, a2 = 0.0f;              // resonator coefficients
    float gainComp = 1.0f;                   // = sin(w), normalises excitation
    float y1 = 0.0f, y2 = 0.0f;              // output history
    float pending = 0.0f;                    // impulse to inject next sample
};
}
