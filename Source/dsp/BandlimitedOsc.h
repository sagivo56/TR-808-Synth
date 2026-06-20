#pragma once

namespace tr808::dsp
{
//==============================================================================
// Anti-aliased oscillator: sine / triangle / square.
//
// The square uses PolyBLEP to band-limit its step discontinuities; the triangle
// is the leaky integral of that band-limited square. Phase resets to 0 on reset()
// (call it on a voice trigger) so every hit starts identically.
//==============================================================================
class BandlimitedOsc
{
public:
    enum class Waveform { sine, triangle, square };

    void prepare (double sampleRate);
    void reset();                                  // phase -> 0

    void setFrequency (float hz) noexcept;
    void setWaveform (Waveform w) noexcept { waveform = w; }

    float processSample() noexcept;

private:
    static float polyBlep (double t, double dt) noexcept;

    double sampleRate = 44100.0;
    double phase      = 0.0;     // normalised 0..1
    double phaseInc   = 0.0;     // cycles per sample
    float  frequency  = 440.0f;
    Waveform waveform = Waveform::sine;

    float triState = 0.0f;       // integrator state for the triangle
};
}
