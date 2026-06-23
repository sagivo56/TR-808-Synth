#pragma once

#include <array>
#include <vector>

namespace tr808::dsp
{
//==============================================================================
// Lush algorithmic reverb (Lexicon-flavoured *sound*, not a UI clone): an
// 8-line feedback delay network with a unitary Hadamard mix (energy-preserving
// => smooth, dense tail), per-line HF damping, a separate bass-vs-mid decay
// (bass multiplier + crossover, like the 224's BASS/MID reverb time), input
// predelay, and gentle per-line modulation (DEPTH) for the chorused tail.
//
// Mono in -> stereo out (the wet signal only; mix/return is handled by the
// caller). Real-time safe: all buffers are allocated in prepare().
//==============================================================================
class ReverbLexicon
{
public:
    void prepare (double sampleRate);
    void reset();

    // All setters are cheap; call per block from the parameters.
    void setPredelay  (float ms) noexcept;        // 0..120 ms
    void setDecay     (float seconds) noexcept;   // mid RT60, ~0.2..12 s
    void setBassMult  (float mult) noexcept;      // bass RT = decay * mult (0.25..2.5)
    void setCrossover (float hz) noexcept;        // bass/mid split (150..2000 Hz)
    void setDamping   (float amount01) noexcept;  // HF roll-off in the tail (0..1)
    void setDepth     (float amount01) noexcept;  // modulation depth (0..1)

    void process (const float* monoIn, float* outL, float* outR, int numSamples) noexcept;

private:
    static constexpr int kLines = 8;

    struct Line
    {
        std::vector<float> buf;
        int   mask = 0;
        int   w    = 0;
        float baseLen = 0.0f;     // delay length in samples
        float dampState = 0.0f;   // HF damping one-pole
        float bassState = 0.0f;   // bass-split one-pole
        float gMid = 0.0f, gBass = 0.0f;
        float lfoPhase = 0.0f, lfoInc = 0.0f;

        void  prepareBuf (int maxLen);
        void  write (float x) noexcept { buf[(size_t) w] = x; w = (w + 1) & mask; }
        float readFrac (float d) const noexcept;
    };

    void recalcGains() noexcept;

    double sampleRate = 44100.0;
    std::array<Line, kLines> lines;

    std::vector<float> predelayBuf;
    int   pdMask = 0, pdW = 0;
    int   predelaySamples = 0;

    float decaySec   = 2.2f;
    float bassMult   = 1.4f;
    float crossover  = 500.0f;
    float damping    = 0.35f;
    float depth      = 0.3f;

    float dampCoef   = 0.0f;   // from damping
    float bassCoef   = 0.0f;   // one-pole coef from crossover
    float modSamples = 0.0f;   // from depth
};
}
