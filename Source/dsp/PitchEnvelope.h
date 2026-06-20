#pragma once

namespace tr808::dsp
{
//==============================================================================
// Exponentially-decaying pitch envelope: jumps to 'amount' on trigger() and
// decays toward 0 over 'time'. The unit of 'amount' is whatever the caller
// wants (semitones or Hz) — this just produces the decaying offset.
//==============================================================================
class PitchEnvelope
{
public:
    void prepare (double sampleRate);
    void reset();

    void setAmount (float amount) noexcept { amountValue = amount; }
    void setTime (float ms) noexcept;

    void trigger() noexcept { value = amountValue; active = (amountValue != 0.0f); }

    bool  isActive() const noexcept { return active; }
    float processSample() noexcept;

private:
    double sampleRate = 44100.0;
    float  amountValue = 0.0f;
    float  value       = 0.0f;
    float  decayMul    = 0.0f;
    float  timeMs      = 50.0f;
    bool   active      = false;
};
}
