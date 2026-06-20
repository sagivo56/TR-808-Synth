#pragma once

namespace tr808::dsp
{
//==============================================================================
// Analog-style envelope with a linear attack and an exponential decay/release.
//
//  - AD mode: trigger() -> attack to 1 -> exponential decay to silence (drums).
//  - AR mode: trigger() -> attack -> hold at 1 -> noteOff() -> exponential release.
//
// Retrigger is click-free: trigger() keeps the current level and ramps up from
// there rather than snapping to zero.
//==============================================================================
class Envelope
{
public:
    enum class Mode { ad, ar };

    void prepare (double sampleRate);
    void reset();                                  // hard reset to 0, inactive

    void setMode  (Mode m) noexcept { mode = m; }
    void setAttack (float ms) noexcept;
    void setDecay  (float ms) noexcept;            // AD decay-to-silence / AR release

    void trigger() noexcept;                       // gate on
    void noteOff() noexcept;                        // gate off (AR only)
    void forceRelease (float ms) noexcept;          // start a fast exp release from the current level (choke)

    bool  isActive() const noexcept { return stage != Stage::idle; }
    float getLevel() const noexcept { return level; }
    float processSample() noexcept;

private:
    enum class Stage { idle, attack, decay, sustain, release };

    double sampleRate = 44100.0;
    Mode   mode  = Mode::ad;
    Stage  stage = Stage::idle;

    float level      = 0.0f;
    float attackInc  = 1.0f;     // linear increment per sample
    float decayMul   = 0.0f;     // exponential multiplier per sample
    float releaseMul = 0.0f;

    float attackMs = 1.0f, decayMs = 200.0f;

    static constexpr float floorLevel = 0.0002f;   // ~ -74 dB end-of-decay threshold
};
}
