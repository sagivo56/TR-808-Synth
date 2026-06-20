#pragma once

namespace tr808::dsp
{
//==============================================================================
// Topology-preserving (Zavalishin/Cytomic) state-variable filter.
// Computes LP/HP/BP from the same structure and stays stable under fast
// cutoff/resonance modulation, which the voices rely on.
//==============================================================================
class SVFilter
{
public:
    enum class Type { lowpass, highpass, bandpass };

    void prepare (double sampleRate);
    void reset();

    void setType      (Type t) noexcept { type = t; }
    void setCutoff    (float hz) noexcept;
    void setResonance (float q) noexcept;

    float processSample (float x) noexcept;

private:
    void updateCoefficients() noexcept;

    double sampleRate = 44100.0;
    Type   type   = Type::lowpass;
    float  cutoff = 1000.0f;
    float  q      = 0.7071f;

    float g = 0.0f, k = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
    float ic1eq = 0.0f, ic2eq = 0.0f;   // integrator states
};
}
