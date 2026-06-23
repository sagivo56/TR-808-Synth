#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace tr808::dsp
{
//==============================================================================
// Mono-in / stereo-out ping-pong delay: the input lands on the left line, each
// line feeds the other through the feedback gain (so repeats bounce L<->R), and
// a one-pole low-pass in the feedback path (TONE) darkens successive repeats.
// Wet only; the return level is handled by the caller. Real-time safe.
//==============================================================================
class StereoDelay
{
public:
    void prepare (double sr)
    {
        sampleRate = sr > 0.0 ? sr : 44100.0;
        const int n = nextPow2 ((int) (2.5 * sampleRate));   // up to 2.5 s
        bufL.assign ((size_t) n, 0.0f);
        bufR.assign ((size_t) n, 0.0f);
        mask = n - 1;
        reset();
        setTime (350.0f);
        setTone (0.5f);
    }

    void reset()
    {
        std::fill (bufL.begin(), bufL.end(), 0.0f);
        std::fill (bufR.begin(), bufR.end(), 0.0f);
        w = 0; lpL = lpR = 0.0f;
    }

    void setTime     (float ms) noexcept     { timeSamples = std::clamp (ms, 1.0f, 2400.0f) * 0.001f * (float) sampleRate; }
    void setFeedback (float fb) noexcept     { feedback = std::clamp (fb, 0.0f, 0.95f); }
    void setTone     (float t01) noexcept    { toneCoef = 0.06f + 0.9f * std::clamp (t01, 0.0f, 1.0f); }   // 1 = bright

    void process (const float* in, float* outL, float* outR, int numSamples) noexcept
    {
        for (int n = 0; n < numSamples; ++n)
        {
            const float dl = readFrac (bufL, timeSamples);
            const float dr = readFrac (bufR, timeSamples);

            // tone: one-pole LP on the delayed signal before it feeds back
            lpL += toneCoef * (dl - lpL);
            lpR += toneCoef * (dr - lpR);

            bufL[(size_t) w] = in[n] + lpR * feedback;   // input + right feedback -> left
            bufR[(size_t) w] = lpL * feedback;           // left feedback -> right
            w = (w + 1) & mask;

            outL[n] = dl;
            outR[n] = dr;
        }
    }

private:
    static int nextPow2 (int n) { int p = 1; while (p < n) p <<= 1; return p; }

    float readFrac (const std::vector<float>& b, float d) const noexcept
    {
        const int   i = (int) d;
        const float f = d - (float) i;
        const float a = b[(size_t) ((w - i)     & mask)];
        const float c = b[(size_t) ((w - i - 1) & mask)];
        return a + (c - a) * f;
    }

    double sampleRate = 44100.0;
    std::vector<float> bufL, bufR;
    int   mask = 0, w = 0;
    float timeSamples = 0.0f, feedback = 0.35f, toneCoef = 0.5f;
    float lpL = 0.0f, lpR = 0.0f;
};
}
