#include "ReverbLexicon.h"

#include <cmath>
#include <algorithm>

namespace tr808::dsp
{
namespace
{
    // Spread, mutually-detuned base delay lengths (samples @ 44.1 kHz): a large,
    // dense hall. Scaled to the actual sample rate in prepare().
    constexpr float kBaseLen44[8] = { 1559.0f, 1907.0f, 2273.0f, 2641.0f,
                                      3001.0f, 3389.0f, 3727.0f, 4093.0f };
    // Slightly different modulation rates per line (Hz) for a non-periodic tail.
    constexpr float kModRate[8]   = { 0.71f, 0.93f, 0.51f, 1.13f, 0.61f, 0.83f, 1.01f, 0.43f };
    // Input injection and L/R output taps (decorrelated sign patterns).
    constexpr float kIn[8]  = {  1.0f, -1.0f,  1.0f, -1.0f,  1.0f, -1.0f,  1.0f, -1.0f };
    constexpr float kOutL[8]= {  1.0f, -1.0f,  1.0f, -1.0f,  1.0f, -1.0f,  1.0f, -1.0f };
    constexpr float kOutR[8]= {  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f };

    inline int nextPow2 (int n) { int p = 1; while (p < n) p <<= 1; return p; }
}

void ReverbLexicon::Line::prepareBuf (int maxLen)
{
    const int n = nextPow2 (maxLen + 4);
    buf.assign ((size_t) n, 0.0f);
    mask = n - 1;
    w = 0;
    dampState = bassState = 0.0f;
}

float ReverbLexicon::Line::readFrac (float d) const noexcept
{
    const int   i = (int) d;
    const float f = d - (float) i;
    const float a = buf[(size_t) ((w - i)     & mask)];
    const float b = buf[(size_t) ((w - i - 1) & mask)];
    return a + (b - a) * f;
}

void ReverbLexicon::prepare (double sr)
{
    sampleRate = sr > 0.0 ? sr : 44100.0;
    const float scale = (float) (sampleRate / 44100.0);

    for (int i = 0; i < kLines; ++i)
    {
        lines[(size_t) i].baseLen = kBaseLen44[i] * scale;
        lines[(size_t) i].prepareBuf ((int) lines[(size_t) i].baseLen + 64);
        lines[(size_t) i].lfoPhase = (float) i * 0.37f;
        lines[(size_t) i].lfoInc   = (float) (2.0 * 3.14159265358979 * kModRate[i] / sampleRate);
    }

    const int pdN = nextPow2 ((int) (0.2 * sampleRate));   // up to 200 ms predelay headroom
    predelayBuf.assign ((size_t) pdN, 0.0f);
    pdMask = pdN - 1; pdW = 0;

    setCrossover (crossover);
    setDamping (damping);
    setDepth (depth);
    recalcGains();
    reset();
}

void ReverbLexicon::reset()
{
    for (auto& l : lines) { std::fill (l.buf.begin(), l.buf.end(), 0.0f); l.w = 0; l.dampState = l.bassState = 0.0f; }
    std::fill (predelayBuf.begin(), predelayBuf.end(), 0.0f);
    pdW = 0;
}

void ReverbLexicon::recalcGains() noexcept
{
    for (auto& l : lines)
    {
        const float lenSec = l.baseLen / (float) sampleRate;
        const float tMid = std::max (0.05f, decaySec);
        const float tBass = std::max (0.05f, decaySec * bassMult);
        l.gMid  = std::pow (10.0f, -3.0f * lenSec / tMid);
        l.gBass = std::pow (10.0f, -3.0f * lenSec / tBass);
    }
}

void ReverbLexicon::setPredelay (float ms) noexcept
{
    const int s = (int) (std::max (0.0f, ms) * 0.001f * (float) sampleRate);
    predelaySamples = std::min (s, pdMask - 1);
}

void ReverbLexicon::setDecay     (float seconds) noexcept { decaySec = seconds; recalcGains(); }
void ReverbLexicon::setBassMult  (float mult)    noexcept { bassMult = mult;    recalcGains(); }

void ReverbLexicon::setCrossover (float hz) noexcept
{
    crossover = hz;
    const float x = std::exp (-2.0f * 3.14159265358979f * std::max (20.0f, hz) / (float) sampleRate);
    bassCoef = x;                       // one-pole LP coefficient for the bass split
}

void ReverbLexicon::setDamping (float a) noexcept
{
    damping = std::clamp (a, 0.0f, 1.0f);
    dampCoef = 0.05f + 0.55f * damping;   // more damping => darker tail
}

void ReverbLexicon::setDepth (float a) noexcept
{
    depth = std::clamp (a, 0.0f, 1.0f);
    modSamples = depth * 12.0f * (float) (sampleRate / 44100.0);
}

void ReverbLexicon::process (const float* in, float* outL, float* outR, int numSamples) noexcept
{
    for (int n = 0; n < numSamples; ++n)
    {
        // predelay
        predelayBuf[(size_t) pdW] = in[n];
        const int rd = (pdW - predelaySamples) & pdMask;
        const float x = predelayBuf[(size_t) rd];
        pdW = (pdW + 1) & pdMask;

        float s[kLines], gv[kLines];
        for (int i = 0; i < kLines; ++i)
        {
            auto& l = lines[(size_t) i];
            const float mod = modSamples * std::sin (l.lfoPhase);
            l.lfoPhase += l.lfoInc;
            s[i] = l.readFrac (l.baseLen + mod);

            // HF damping (one-pole LP)
            l.dampState += dampCoef * (s[i] - l.dampState);
            float d = l.dampState;

            // bass/mid split decay: extra gain on the low band
            l.bassState += (1.0f - bassCoef) * (d - l.bassState);
            const float low = l.bassState;
            gv[i] = l.gMid * d + (l.gBass - l.gMid) * low;
        }

        // unitary Hadamard mix (fast Walsh-Hadamard, scaled by 1/sqrt(8))
        for (int len = 1; len < kLines; len <<= 1)
            for (int i = 0; i < kLines; i += len << 1)
                for (int j = i; j < i + len; ++j)
                {
                    const float a = gv[j], b = gv[j + len];
                    gv[j] = a + b; gv[j + len] = a - b;
                }

        float oL = 0.0f, oR = 0.0f;
        for (int i = 0; i < kLines; ++i)
        {
            lines[(size_t) i].write (x * kIn[i] + gv[i] * 0.35355339f);
            oL += kOutL[i] * s[i];
            oR += kOutR[i] * s[i];
        }

        outL[n] = oL * 0.35f;
        outR[n] = oR * 0.35f;
    }
}
}
