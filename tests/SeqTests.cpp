// Sequencer tests for M4.
//
// Drives the Sequencer with host and internal transport and checks step timing
// (sample-accurate from PPQ), the default beat, accent, mute/solo, swing and a
// state round-trip. Console app: exits non-zero on any failure.

#include "engine/Sequencer.h"
#include "engine/VoiceDefs.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

using namespace tr808;

namespace
{
int g_total = 0, g_failed = 0;

void check (bool cond, const std::string& name, const std::string& detail = {})
{
    ++g_total;
    if (cond) { std::cout << "  [pass] " << name << "\n"; }
    else      { ++g_failed; std::cout << "  [FAIL] " << name; if (! detail.empty()) std::cout << "   (" << detail << ")"; std::cout << "\n"; }
}

constexpr double kSr = 44100.0;

int countVoice (const std::vector<TriggerEvent>& ev, int voice)
{
    return (int) std::count_if (ev.begin(), ev.end(), [voice] (const TriggerEvent& e) { return e.voiceIndex == voice; });
}

const TriggerEvent* firstOf (const std::vector<TriggerEvent>& ev, int voice)
{
    for (const auto& e : ev) if (e.voiceIndex == voice) return &e;
    return nullptr;
}

// Run the internal clock for 'seconds', returning events with absolute sample positions.
std::vector<TriggerEvent> runInternal (Sequencer& s, double seconds, int block)
{
    std::vector<TriggerEvent> all;
    const int total = (int) std::lround (seconds * kSr);
    int done = 0;
    while (done < total)
    {
        const int n = std::min (block, total - done);
        Sequencer::TransportInfo t; t.hostPlaying = false; t.sampleRate = kSr; t.numSamples = n;
        std::vector<TriggerEvent> ev;
        s.process (t, ev);
        for (auto e : ev) { e.samplePos += done; all.push_back (e); }
        done += n;
    }
    return all;
}
} // namespace

static void testDefaultBeatHostSync()
{
    std::cout << "Default beat / host sync\n";
    Sequencer s; s.prepare (kSr);

    // One beat: ppq [0, 1.0) at 120 BPM = 22050 samples -> steps 0..3.
    Sequencer::TransportInfo t; t.hostPlaying = true; t.bpm = 120.0; t.ppqPosition = 0.0; t.sampleRate = kSr; t.numSamples = 22050;
    std::vector<TriggerEvent> ev;
    s.process (t, ev);

    check (countVoice (ev, BD) == 1, "BD fires once in beat 1 (step 0)", "n=" + std::to_string (countVoice (ev, BD)));
    check (countVoice (ev, CH) == 2, "CH fires on steps 0 and 2", "n=" + std::to_string (countVoice (ev, CH)));
    if (const auto* bd = firstOf (ev, BD))
    {
        check (bd->samplePos <= 4, "BD step 0 is at the block start", "pos=" + std::to_string (bd->samplePos));
        check (bd->accent, "step 0 is accented");
    }
    else check (false, "BD event present");

    // A step mid-block lands at the right sample (step 2 @ ppq 0.5 -> ~11025).
    const int expected = (int) std::lround (0.5 / ((120.0 / 60.0) / kSr));
    bool found = false;
    for (const auto& e : ev) if (e.voiceIndex == CH && std::abs (e.samplePos - expected) <= 2) found = true;
    check (found, "step 2 is sample-accurate (~11025)", "expected=" + std::to_string (expected));
}

static void testNoDoubleFireAcrossBlocks()
{
    std::cout << "Contiguous blocks: no double-fire\n";
    Sequencer s; s.prepare (kSr);

    auto run = [&] (double ppqStart, int n)
    {
        Sequencer::TransportInfo t; t.hostPlaying = true; t.bpm = 120.0; t.ppqPosition = ppqStart; t.sampleRate = kSr; t.numSamples = n;
        std::vector<TriggerEvent> ev; s.process (t, ev); return ev;
    };
    auto a = run (0.0, 11025);     // ppq [0, 0.5)  -> step 0
    auto b = run (0.5, 11025);     // ppq [0.5,1.0) -> step 2
    check (countVoice (a, BD) == 1 && countVoice (b, BD) == 0, "BD only in the block containing step 0");
    check (countVoice (a, CH) == 1 && countVoice (b, CH) == 1, "CH split across the two blocks, once each");
}

static void testInternalTransport()
{
    std::cout << "Internal transport\n";
    Sequencer s; s.prepare (kSr);
    s.setTempo (120.0);

    // Not running -> silent.
    s.setRunning (false);
    check (runInternal (s, 0.5, 512).empty(), "no events while stopped");

    // Running: just under one bar (1.9 s -> ppq ~3.8, steps 0..15) keeps the
    // window off the exact ppq=4.0 boundary. BD on 0,4,8,12 = 4; SD on 4,12 = 2.
    s.setRunning (true);
    auto ev = runInternal (s, 1.9, 512);
    check (countVoice (ev, BD) == 4, "BD fires 4x per bar via internal clock", "n=" + std::to_string (countVoice (ev, BD)));
    check (countVoice (ev, SD) == 2, "SD fires 2x per bar (steps 4,12)", "n=" + std::to_string (countVoice (ev, SD)));
}

static void testMuteSolo()
{
    std::cout << "Mute / solo\n";
    {
        Sequencer s; s.prepare (kSr); s.setTempo (120.0); s.setRunning (true);
        s.setMute (BD, true);
        auto ev = runInternal (s, 2.0, 512);
        check (countVoice (ev, BD) == 0, "muted BD produces nothing");
        check (countVoice (ev, CH) > 0, "other voices still play when BD muted");
    }
    {
        Sequencer s; s.prepare (kSr); s.setTempo (120.0); s.setRunning (true);
        s.setSolo (CH, true);
        auto ev = runInternal (s, 2.0, 512);
        check (countVoice (ev, CH) > 0 && countVoice (ev, BD) == 0 && countVoice (ev, SD) == 0,
               "solo CH silences non-soloed voices");
    }
}

static void testSwing()
{
    std::cout << "Swing\n";
    Sequencer s; s.prepare (kSr);
    s.setStep (0, 0, MA, 1, true);    // offbeat 16th
    s.setSwing (0.5f);

    Sequencer::TransportInfo t; t.hostPlaying = true; t.bpm = 120.0; t.ppqPosition = 0.0; t.sampleRate = kSr; t.numSamples = 11025;
    std::vector<TriggerEvent> ev; s.process (t, ev);

    const int grid = (int) std::lround (0.25 / ((120.0 / 60.0) / kSr));   // ~5512
    if (const auto* ma = firstOf (ev, MA))
        check (ma->samplePos > grid + 500, "offbeat step is delayed by swing", "pos=" + std::to_string (ma->samplePos) + " grid=" + std::to_string (grid));
    else
        check (false, "MA offbeat event present");
}

static void testStateRoundTrip()
{
    std::cout << "State round-trip\n";
    Sequencer s1; s1.prepare (kSr);
    s1.setStep (2, 1, SD, 5, true);
    s1.setStep (2, 1, OH, 9, true);

    const auto vt = s1.toValueTree();
    Sequencer s2; s2.prepare (kSr);
    s2.fromValueTree (vt);

    check (s2.getStep (2, 1, SD, 5), "saved step (pat2/varB/SD/5) restored");
    check (s2.getStep (2, 1, OH, 9), "saved step (pat2/varB/OH/9) restored");
    check (s2.getStep (0, 0, BD, 0), "default-beat step (BD step 0) restored");
    check (! s2.getStep (2, 1, SD, 6), "unset step stays off");
}

int main()
{
    std::cout << "=== TR-808 sequencer tests (sr=" << kSr << ") ===\n";

    testDefaultBeatHostSync();
    testNoDoubleFireAcrossBlocks();
    testInternalTransport();
    testMuteSolo();
    testSwing();
    testStateRoundTrip();

    std::cout << "\n" << (g_total - g_failed) << "/" << g_total << " checks passed.\n";
    if (g_failed > 0) { std::cout << "*** " << g_failed << " FAILED ***\n"; return 1; }
    std::cout << "ALL PASSED\n";
    return 0;
}
