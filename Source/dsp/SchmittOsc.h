#pragma once

namespace tr808::dsp
{
//==============================================================================
// Circuit model of the TR-808's metallic-voice oscillators: a CMOS
// Schmitt-trigger (40106-style) inverter wired as an RC relaxation oscillator.
//
// A capacitor charges toward the inverter's output rail through R; the output
// flips when the cap voltage crosses the Schmitt hysteresis thresholds. That
// reproduces the real square's slightly soft edges and duty, rather than an
// ideal mathematical square. Internally oversampled and box-averaged so the
// hard comparator flips don't alias badly; the finite output slew models the
// inverter's edge rate.
//==============================================================================
class SchmittOsc
{
public:
    void prepare (double sampleRate);
    void reset();

    void setFrequency (float hz) noexcept;

    float processSample() noexcept;

private:
    static constexpr int kOversample = 8;

    double sampleRate = 44100.0;
    float  rate    = 0.0f;       // per oversampled-step RC charge coefficient
    float  vcap    = 0.0f;       // capacitor voltage
    float  out     = 1.0f;       // comparator output (+/-1)
    float  outSlew = 0.0f;       // slew-limited output (finite edge)
    float  slewCoef = 0.5f;

    static constexpr float thHi =  0.5f;   // Schmitt thresholds
    static constexpr float thLo = -0.5f;
};
}
