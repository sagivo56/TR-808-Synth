// Offline DSP test harness for the TR-808 building blocks.
//
// Renders each /dsp block and checks it numerically (level, decay, stability)
// and spectrally (juce::dsp::FFT) — most importantly that the PolyBLEP square is
// band-limited. Console app: prints per-check pass/fail and exits non-zero on
// any failure.

#include <juce_dsp/juce_dsp.h>

#include "dsp/BandlimitedOsc.h"
#include "dsp/NoiseGen.h"
#include "dsp/Envelope.h"
#include "dsp/PitchEnvelope.h"
#include "dsp/SVFilter.h"
#include "dsp/ResonatorBT.h"
#include "dsp/Saturator.h"
#include "dsp/MetalCluster.h"
#include "dsp/SchmittOsc.h"
#include "dsp/ReverbLexicon.h"
#include "dsp/StereoDelay.h"

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include <algorithm>

using namespace tr808::dsp;

//==============================================================================
namespace
{
int g_total = 0;
int g_failed = 0;

void check (bool cond, const std::string& name, const std::string& detail = {})
{
    ++g_total;
    if (cond)
    {
        std::cout << "  [pass] " << name << "\n";
    }
    else
    {
        ++g_failed;
        std::cout << "  [FAIL] " << name;
        if (! detail.empty()) std::cout << "   (" << detail << ")";
        std::cout << "\n";
    }
}

constexpr double kSampleRate = 44100.0;
constexpr int    kFftOrder   = 14;          // 16384-point FFT
constexpr int    kFftSize    = 1 << kFftOrder;
constexpr double kBinHz      = kSampleRate / (double) kFftSize;

std::vector<float> magSpectrum (const std::vector<float>& sig)
{
    juce::dsp::FFT fft (kFftOrder);
    std::vector<float> buf (2 * (size_t) kFftSize, 0.0f);
    const int count = std::min ((int) sig.size(), kFftSize);
    for (int i = 0; i < count; ++i) buf[(size_t) i] = sig[(size_t) i];
    fft.performFrequencyOnlyForwardTransform (buf.data());
    return { buf.begin(), buf.begin() + kFftSize / 2 };
}

double rms (const std::vector<float>& v, int start, int count)
{
    double acc = 0.0;
    int n = 0;
    for (int i = start; i < start + count && i < (int) v.size(); ++i) { acc += (double) v[(size_t) i] * v[(size_t) i]; ++n; }
    return n > 0 ? std::sqrt (acc / n) : 0.0;
}

int dominantBin (const std::vector<float>& mag, int loBin = 1)
{
    int best = loBin; float bestV = -1.0f;
    for (int i = loBin; i < (int) mag.size(); ++i)
        if (mag[(size_t) i] > bestV) { bestV = mag[(size_t) i]; best = i; }
    return best;
}

bool allFinite (const std::vector<float>& v, float bound)
{
    for (float x : v)
        if (! std::isfinite (x) || std::abs (x) > bound) return false;
    return true;
}
} // namespace

//==============================================================================
static void testBandlimitedOsc()
{
    std::cout << "BandlimitedOsc\n";

    // Choose a frequency that is an exact integer number of cycles in the FFT
    // window (1000 cycles) so harmonics land exactly on bins -> no leakage.
    const int    cycles = 1000;
    const double f0     = cycles * kBinHz;     // ~2691.6 Hz

    // --- Sine: single spectral line ---
    {
        BandlimitedOsc osc; osc.prepare (kSampleRate);
        osc.setWaveform (BandlimitedOsc::Waveform::sine);
        osc.setFrequency ((float) f0);
        std::vector<float> sig (kFftSize);
        for (auto& s : sig) s = osc.processSample();

        auto mag = magSpectrum (sig);
        const int peak = dominantBin (mag);
        double fund = 0.0, rest = 0.0;
        for (int b = 3; b < (int) mag.size(); ++b)
        {
            const double p = (double) mag[(size_t) b] * mag[(size_t) b];
            if (std::abs (b - cycles) <= 2) fund += p; else rest += p;
        }
        check (peak == cycles, "sine peak at fundamental bin", "peak=" + std::to_string (peak));
        check (rest / fund < 1.0e-3, "sine is spectrally pure (THD+alias < -30 dB)",
               "ratio=" + std::to_string (rest / fund));
    }

    // --- Square: PolyBLEP must be far less aliased than a naive square ---
    {
        BandlimitedOsc osc; osc.prepare (kSampleRate);
        osc.setWaveform (BandlimitedOsc::Waveform::square);
        osc.setFrequency ((float) f0);
        std::vector<float> blep (kFftSize);
        for (auto& s : blep) s = osc.processSample();

        std::vector<float> naive (kFftSize);
        double phase = 0.0; const double inc = f0 / kSampleRate;
        for (auto& s : naive) { s = (phase < 0.5 ? 1.0f : -1.0f); phase += inc; if (phase >= 1.0) phase -= 1.0; }

        auto aliasRatio = [] (const std::vector<float>& sig)
        {
            auto mag = magSpectrum (sig);
            auto isHarmonic = [] (int b)
            { for (int k : { 1000, 3000, 5000, 7000 }) if (std::abs (b - k) <= 2) return true; return false; };
            double hp = 0.0, ap = 0.0;
            for (int b = 3; b < (int) mag.size(); ++b)
            {
                const double p = (double) mag[(size_t) b] * mag[(size_t) b];
                if (isHarmonic (b)) hp += p; else ap += p;
            }
            return ap / hp;
        };

        const double rBlep  = aliasRatio (blep);
        const double rNaive = aliasRatio (naive);
        check (rBlep < rNaive * 0.5, "PolyBLEP square less aliased than naive",
               "blep=" + std::to_string (rBlep) + " naive=" + std::to_string (rNaive));
        check (rBlep < 0.05, "PolyBLEP square alias energy is low (< -13 dB)",
               "ratio=" + std::to_string (rBlep));
    }

    // --- Triangle: oscillates and peaks at its fundamental ---
    {
        const int triCycles = 200;
        BandlimitedOsc osc; osc.prepare (kSampleRate);
        osc.setWaveform (BandlimitedOsc::Waveform::triangle);
        osc.setFrequency ((float) (triCycles * kBinHz));
        std::vector<float> sig (kFftSize);
        float mn = 1e9f, mx = -1e9f;
        for (auto& s : sig) { s = osc.processSample(); mn = std::min (mn, s); mx = std::max (mx, s); }
        auto mag = magSpectrum (sig);
        check (mx > 0.3f && mn < -0.3f, "triangle oscillates", "min=" + std::to_string (mn) + " max=" + std::to_string (mx));
        check (std::abs (dominantBin (mag) - triCycles) <= 2, "triangle peak at fundamental");
    }
}

static void testNoiseGen()
{
    std::cout << "NoiseGen\n";
    const int n = kFftSize;

    NoiseGen ng; ng.prepare (kSampleRate); ng.setSeed (12345);
    std::vector<float> white (n);
    double sum = 0.0;
    for (auto& s : white) { s = ng.processSample(); sum += s; }
    check (allFinite (white, 1.0001f), "white noise bounded to [-1,1]");
    check (std::abs (sum / n) < 0.05, "white noise mean ~ 0", "mean=" + std::to_string (sum / n));
    check (rms (white, 0, n) > 0.2, "white noise has energy");

    // Low-pass shaping must remove high-frequency energy (same seed -> comparable).
    auto hfFraction = [&] (bool lp)
    {
        ng.reset(); ng.setSeed (777);
        ng.setLowpass (lp ? 500.0f : 0.0f);
        ng.setHighpass (0.0f);
        std::vector<float> sig (n);
        for (auto& s : sig) s = ng.processSample();
        auto mag = magSpectrum (sig);
        double hi = 0.0, total = 0.0;
        const int hiBin = (int) (5000.0 / kBinHz);
        for (int b = 1; b < (int) mag.size(); ++b)
        {
            const double p = (double) mag[(size_t) b] * mag[(size_t) b];
            total += p; if (b >= hiBin) hi += p;
        }
        return hi / total;
    };
    const double fOff = hfFraction (false);
    const double fOn  = hfFraction (true);
    check (fOn < fOff * 0.5, "low-pass reduces high-frequency content",
           "off=" + std::to_string (fOff) + " on=" + std::to_string (fOn));
}

static void testEnvelope()
{
    std::cout << "Envelope\n";

    // AD shape
    {
        Envelope env; env.prepare (kSampleRate);
        env.setMode (Envelope::Mode::ad);
        env.setAttack (1.0f); env.setDecay (100.0f);
        env.trigger();

        const int n = (int) kSampleRate;        // 1 s
        std::vector<float> sig (n);
        float peak = 0.0f;
        for (auto& s : sig) { s = env.processSample(); peak = std::max (peak, s); }

        check (peak >= 0.98f && peak <= 1.0001f, "AD reaches ~unity peak", "peak=" + std::to_string (peak));
        check (sig[(size_t) (0.15 * kSampleRate)] < 0.02f, "AD decays to silence by 150 ms");
        check (! env.isActive(), "AD becomes inactive after decay");

        bool monotonic = true;
        const int peakIdx = (int) std::distance (sig.begin(), std::max_element (sig.begin(), sig.end()));
        for (int i = peakIdx + 1; i < n; ++i) if (sig[(size_t) i] > sig[(size_t) (i - 1)] + 1.0e-6f) { monotonic = false; break; }
        check (monotonic, "AD decay is monotonic");
    }

    // Click-free retrigger: no large sample-to-sample jump when re-triggered mid-decay
    {
        Envelope env; env.prepare (kSampleRate);
        env.setAttack (5.0f); env.setDecay (200.0f);
        std::vector<float> sig;
        env.trigger();
        for (int i = 0; i < 2000; ++i) sig.push_back (env.processSample());
        env.trigger();                                   // retrigger during decay
        for (int i = 0; i < 2000; ++i) sig.push_back (env.processSample());

        float maxDelta = 0.0f;
        for (size_t i = 1; i < sig.size(); ++i) maxDelta = std::max (maxDelta, std::abs (sig[i] - sig[i - 1]));
        check (maxDelta < 0.02f, "retrigger is click-free (no level jump)", "maxDelta=" + std::to_string (maxDelta));
    }
}

static void testPitchEnvelope()
{
    std::cout << "PitchEnvelope\n";
    PitchEnvelope pe; pe.prepare (kSampleRate);
    pe.setAmount (12.0f); pe.setTime (50.0f);
    pe.trigger();

    const int n = (int) kSampleRate;
    std::vector<float> sig (n);
    for (auto& s : sig) s = pe.processSample();

    check (sig[0] > 11.5f, "pitch env starts at amount", "v0=" + std::to_string (sig[0]));
    check (sig[(size_t) (0.2 * kSampleRate)] < 1.0f, "pitch env decays toward 0");
    bool monotonic = true;
    for (int i = 1; i < n; ++i) if (sig[(size_t) i] > sig[(size_t) (i - 1)] + 1.0e-6f) { monotonic = false; break; }
    check (monotonic, "pitch env decay is monotonic");
    check (! pe.isActive(), "pitch env becomes inactive");
}

static void testSVFilter()
{
    std::cout << "SVFilter\n";
    const int n = kFftSize;

    auto sine = [&] (double hz)
    {
        std::vector<float> v (n);
        const double w = 2.0 * 3.14159265358979 * hz / kSampleRate;
        for (int i = 0; i < n; ++i) v[(size_t) i] = (float) std::sin (w * i);
        return v;
    };
    auto runFilter = [&] (const std::vector<float>& in, SVFilter::Type t, float fc, float q)
    {
        SVFilter f; f.prepare (kSampleRate); f.setType (t); f.setResonance (q); f.setCutoff (fc);
        std::vector<float> out (in.size());
        for (size_t i = 0; i < in.size(); ++i) out[i] = f.processSample (in[i]);
        return out;
    };

    // LP attenuates a high tone, passes a low tone
    {
        auto hi = sine (8000.0);
        auto loPass = runFilter (sine (200.0), SVFilter::Type::lowpass, 2000.0f, 0.707f);
        auto hiCut  = runFilter (hi,            SVFilter::Type::lowpass, 500.0f,  0.707f);
        check (rms (hiCut, 2000, n - 2000) < rms (hi, 2000, n - 2000) * 0.2, "LP attenuates 8 kHz tone");
        check (rms (loPass, 2000, n - 2000) > rms (sine (200.0), 2000, n - 2000) * 0.7, "LP passes 200 Hz tone");
    }

    // HP attenuates a low tone
    {
        auto lo = sine (100.0);
        auto cut = runFilter (lo, SVFilter::Type::highpass, 1000.0f, 0.707f);
        check (rms (cut, 2000, n - 2000) < rms (lo, 2000, n - 2000) * 0.3, "HP attenuates 100 Hz tone");
    }

    // Stability under high resonance / extreme cutoffs with noisy input
    {
        NoiseGen ng; ng.prepare (kSampleRate); ng.setSeed (9);
        std::vector<float> noise (n); for (auto& s : noise) s = ng.processSample();
        bool ok = true;
        for (auto fc : { 50.0f, 15000.0f, 21000.0f })
            for (auto q : { 0.5f, 10.0f, 30.0f })
            {
                auto out = runFilter (noise, SVFilter::Type::bandpass, fc, q);
                if (! allFinite (out, 100.0f)) ok = false;
            }
        check (ok, "SVF stays stable/bounded across cutoff & Q");
    }
}

static void testResonatorBT()
{
    std::cout << "ResonatorBT\n";
    ResonatorBT res; res.prepare (kSampleRate);
    res.setFrequency (200.0f); res.setDecay (0.5f);
    res.excite (1.0f);

    const int n = 22050;                      // 0.5 s
    std::vector<float> sig (n);
    for (auto& s : sig) s = res.processSample();

    auto mag = magSpectrum (sig);
    const int peak = dominantBin (mag, 1);
    const double peakHz = peak * kBinHz;
    check (std::abs (peakHz - 200.0) < 15.0, "resonator rings at set frequency", "peakHz=" + std::to_string (peakHz));
    check (rms (sig, 0, 2205) > rms (sig, n - 2205, 2205) * 2.0, "resonator output decays");
    check (allFinite (sig, 100.0f), "resonator output bounded");
}

static void testSaturator()
{
    std::cout << "Saturator\n";

    check (std::abs (Saturator::shape (-0.5f, 3.0f) + Saturator::shape (0.5f, 3.0f)) < 1.0e-5f, "saturator is odd-symmetric");
    check (Saturator::shape (0.1f, 3.0f) < Saturator::shape (0.3f, 3.0f)
        && Saturator::shape (0.3f, 3.0f) < Saturator::shape (0.6f, 3.0f), "saturator curve is monotonic");
    check (std::abs (Saturator::shape (10.0f, 5.0f)) <= 1.0f, "saturator output is bounded |y|<=1");
    check (Saturator::shape (0.1f, 8.0f) > Saturator::shape (0.1f, 1.0f), "more drive -> more output for small input");

    // Oversampled block runs and stays bounded
    {
        Saturator sat; sat.prepare (kSampleRate, 512, 1, 1); sat.setDrive (6.0f);
        std::vector<float> ch0 (512);
        const double w = 2.0 * 3.14159265358979 * 7000.0 / kSampleRate;
        for (int i = 0; i < 512; ++i) ch0[(size_t) i] = 0.9f * (float) std::sin (w * i);
        float* chans[1] = { ch0.data() };
        juce::dsp::AudioBlock<float> block (chans, 1, 512);
        sat.processBlock (block);
        // The half-band resampling filter rings on the near-square output, so a
        // little overshoot past 1.0 is expected; we only assert it stays sane.
        check (allFinite (ch0, 2.0f), "oversampled saturator block is finite & bounded");
        check (sat.getLatencySamples() >= 0, "oversampler reports latency");
    }
}

static void testMetalCluster()
{
    std::cout << "MetalCluster\n";
    MetalCluster mc; mc.prepare (kSampleRate);
    std::vector<float> sig (kFftSize);
    for (auto& s : sig) s = mc.processSample();

    check (allFinite (sig, 1.5f), "metal cluster output bounded");
    check (rms (sig, 0, kFftSize) > 0.05, "metal cluster has energy");

    auto mag = magSpectrum (sig);
    double mean = 0.0; for (float m : mag) mean += m; mean /= mag.size();
    auto prominentNear = [&] (double hz)
    {
        const int b = (int) std::lround (hz / kBinHz);
        float best = 0.0f;
        for (int i = b - 2; i <= b + 2; ++i) if (i >= 0 && i < (int) mag.size()) best = std::max (best, mag[(size_t) i]);
        return best > mean * 3.0;
    };
    check (prominentNear (205.3), "cluster has a peak near 205 Hz");
    check (prominentNear (800.0), "cluster has a peak near 800 Hz");
}

static void testSchmittOsc()
{
    std::cout << "SchmittOsc (circuit-modeled square)\n";
    const int cycles = 500;
    const double f0 = cycles * kBinHz;          // ~1345 Hz

    SchmittOsc osc; osc.prepare (kSampleRate);
    osc.setFrequency ((float) f0);

    std::vector<float> sig (kFftSize);
    float mn = 1.0e9f, mx = -1.0e9f;
    for (auto& s : sig) { s = osc.processSample(); mn = std::min (mn, s); mx = std::max (mx, s); }

    check (allFinite (sig, 1.5f), "schmitt osc bounded");
    check (mx > 0.3f && mn < -0.3f, "schmitt osc swings like a square", "min=" + std::to_string (mn) + " max=" + std::to_string (mx));
    check (std::abs (dominantBin (magSpectrum (sig)) - cycles) <= 6, "schmitt osc runs at the set frequency");
}

static void testReverb()
{
    std::cout << "ReverbLexicon (FDN)\n";
    ReverbLexicon rev; rev.prepare (kSampleRate);
    rev.setDecay (2.0f); rev.setPredelay (15.0f); rev.setBassMult (1.4f);
    rev.setCrossover (500.0f); rev.setDamping (0.35f); rev.setDepth (0.3f);

    const int N = (int) kSampleRate;             // 1 second
    std::vector<float> in (N, 0.0f), oL (N, 0.0f), oR (N, 0.0f);
    in[0] = 1.0f;
    rev.process (in.data(), oL.data(), oR.data(), N);

    auto rms = [] (const std::vector<float>& v, int a, int b)
    { double s = 0; int n = 0; for (int i = a; i < b && i < (int) v.size(); ++i) { s += (double) v[(size_t) i] * v[(size_t) i]; ++n; } return n ? std::sqrt (s / n) : 0.0; };

    bool finite = true; for (float x : oL) if (! std::isfinite (x) || std::abs (x) > 8.0f) finite = false;
    for (float x : oR) if (! std::isfinite (x) || std::abs (x) > 8.0f) finite = false;

    const double early = rms (oL, 0, (int) (0.15 * kSampleRate));
    const double late  = rms (oL, (int) (0.85 * kSampleRate), N);
    double diff = 0; for (int i = 0; i < N; ++i) diff += std::abs (oL[(size_t) i] - oR[(size_t) i]);

    check (finite, "reverb output finite & bounded");
    check (early > 1.0e-4, "reverb produces a tail", "early=" + std::to_string (early));
    check (late < early * 0.7, "reverb tail decays", "early=" + std::to_string (early) + " late=" + std::to_string (late));
    check (diff > 1.0, "reverb output is stereo (L != R)", "diff=" + std::to_string (diff));
}

static void testDelay()
{
    std::cout << "StereoDelay (ping-pong)\n";
    StereoDelay dly; dly.prepare (kSampleRate);
    dly.setTime (100.0f); dly.setFeedback (0.5f); dly.setTone (0.9f);

    const int N = (int) (0.5 * kSampleRate);
    std::vector<float> in (N, 0.0f), oL (N, 0.0f), oR (N, 0.0f);
    in[0] = 1.0f;
    dly.process (in.data(), oL.data(), oR.data(), N);

    const int t = (int) (0.1 * kSampleRate);     // 100 ms
    auto peak = [] (const std::vector<float>& v, int a, int b)
    { float m = 0; for (int i = a; i < b && i < (int) v.size(); ++i) m = std::max (m, std::abs (v[(size_t) i])); return m; };

    bool finite = true; for (float x : oL) if (! std::isfinite (x)) finite = false;
    check (finite, "delay output finite");
    check (peak (oL, t - 40, t + 40) > 0.3f, "first repeat lands on L at the delay time");
    check (peak (oR, 2 * t - 40, 2 * t + 40) > 0.1f, "second repeat bounces to R");
}

//==============================================================================
int main()
{
    std::cout << "=== TR-808 DSP tests (sr=" << kSampleRate << ") ===\n";

    testBandlimitedOsc();
    testSchmittOsc();
    testNoiseGen();
    testEnvelope();
    testPitchEnvelope();
    testSVFilter();
    testResonatorBT();
    testSaturator();
    testMetalCluster();
    testReverb();
    testDelay();

    std::cout << "\n" << (g_total - g_failed) << "/" << g_total << " checks passed.\n";
    if (g_failed > 0) { std::cout << "*** " << g_failed << " FAILED ***\n"; return 1; }
    std::cout << "ALL PASSED\n";
    return 0;
}
