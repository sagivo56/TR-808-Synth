#include "Sequencer.h"

#include <algorithm>
#include <cmath>

namespace tr808
{
Sequencer::Sequencer()
{
    // A simple default beat in pattern 0 / variation A so something plays.
    auto& a = patterns[0].var[0];
    a.length = 16;
    for (int st : { 0, 4, 8, 12 }) a.steps[(size_t) BD][(size_t) st] = true;
    for (int st : { 4, 12 })       a.steps[(size_t) SD][(size_t) st] = true;
    for (int st = 0; st < 16; st += 2) a.steps[(size_t) CH][(size_t) st] = true;
    a.accent[0] = a.accent[8] = true;
}

void Sequencer::prepare (double sr) { sampleRate = sr; internalPpq = 0.0; }
void Sequencer::reset()             { internalPpq = 0.0; }

bool Sequencer::voicePlayable (int voice) const
{
    const bool anySolo = std::any_of (soloed.begin(), soloed.end(), [] (bool b) { return b; });
    return ! muted[(size_t) voice] && (! anySolo || soloed[(size_t) voice]);
}

float Sequencer::hashUnit (long long step, int voice)
{
    // Deterministic per (step, voice) so probability is resync-safe and a step's
    // main hit and flam agree even across block boundaries.
    uint64_t x = (uint64_t) (step * 131 + voice * 977) + 0x9E3779B97F4A7C15ULL;
    x ^= x >> 30; x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27; x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;
    return (float) ((double) (x >> 40) / (double) (1ULL << 24));
}

void Sequencer::process (const TransportInfo& t, std::vector<TriggerEvent>& out)
{
    out.clear();

    const bool playing = t.hostPlaying || running;
    const double bpm = t.hostPlaying ? (t.bpm > 0.0 ? t.bpm : 120.0) : internalBpm;
    const double sr  = t.sampleRate > 0.0 ? t.sampleRate : sampleRate;
    if (! playing || bpm <= 0.0 || t.numSamples <= 0)
        return;

    const double ppqPerSample = (bpm / 60.0) / sr;
    const double ppqStart = t.hostPlaying ? t.ppqPosition : internalPpq;
    const double ppqEnd   = ppqStart + (double) t.numSamples * ppqPerSample;

    const double stepQ = 0.25;                                    // a 16th note
    const double flamQ = (double) flamMs * 0.001 * (bpm / 60.0);
    const int    refLen = juce::jmax (1, patterns[(size_t) currentPattern].var[0].length);

    const double margin = stepQ + flamQ + (double) swing * stepQ + ppqPerSample;
    long long sLo = (long long) std::floor ((ppqStart - margin) / stepQ);
    long long sHi = (long long) std::ceil (ppqEnd / stepQ) + 1;
    if (sLo < 0) sLo = 0;

    const bool anySolo = std::any_of (soloed.begin(), soloed.end(), [] (bool b) { return b; });

    for (long long s = sLo; s <= sHi; ++s)
    {
        const bool   offbeat     = (s & 1LL) != 0;
        const double swingDelayQ = offbeat ? (double) swing * stepQ * 0.5 : 0.0;
        const double mainPpq     = (double) s * stepQ + swingDelayQ;

        const long long loopIndex  = s / refLen;
        const int       stepInLoop = (int) (s % refLen);

        int patIdx = currentPattern;
        if (chainEnabled && ! chain.empty())
        {
            const long long n = (long long) chain.size();
            patIdx = juce::jlimit (0, numPatterns - 1, chain[(size_t) (((loopIndex % n) + n) % n)]);
        }
        const Pattern& pat = patterns[(size_t) patIdx];
        const int varIdx = (playMode == PlayMode::a) ? 0
                         : (playMode == PlayMode::b) ? 1
                                                     : (int) (loopIndex & 1LL);     // AB
        const Variation& v = pat.var[varIdx];
        if (stepInLoop >= juce::jmax (1, v.length))
            continue;

        const bool mainInBlock = (mainPpq >= ppqStart && mainPpq < ppqEnd);

        for (int vo = 0; vo < numVoices; ++vo)
        {
            if (! v.steps[(size_t) vo][(size_t) stepInLoop])
                continue;
            if (muted[(size_t) vo] || (anySolo && ! soloed[(size_t) vo]))
                continue;
            if (probEnabled && hashUnit (s, vo) > v.prob[(size_t) vo][(size_t) stepInLoop])
                continue;

            if (mainInBlock)
            {
                const int off = juce::jlimit (0, t.numSamples - 1, (int) ((mainPpq - ppqStart) / ppqPerSample));
                out.push_back ({ off, vo, 0.8f, v.accent[(size_t) stepInLoop] });
            }

            if (v.flam[(size_t) vo][(size_t) stepInLoop])
            {
                const double flamPpq = mainPpq + flamQ;
                if (flamPpq >= ppqStart && flamPpq < ppqEnd)
                {
                    const int off = juce::jlimit (0, t.numSamples - 1, (int) ((flamPpq - ppqStart) / ppqPerSample));
                    out.push_back ({ off, vo, 0.55f, false });
                }
            }
        }
    }

    if (! t.hostPlaying)
        internalPpq = ppqEnd;

    std::sort (out.begin(), out.end(),
               [] (const TriggerEvent& a, const TriggerEvent& b) { return a.samplePos < b.samplePos; });
}

//== Editing ===================================================================
void Sequencer::setStep (int pat, int var, int voice, int step, bool on)
{
    if (pat >= 0 && pat < numPatterns && (var == 0 || var == 1) && validVoice (voice) && step >= 0 && step < maxSteps)
        patterns[(size_t) pat].var[var].steps[(size_t) voice][(size_t) step] = on;
}

bool Sequencer::getStep (int pat, int var, int voice, int step) const
{
    if (pat >= 0 && pat < numPatterns && (var == 0 || var == 1) && validVoice (voice) && step >= 0 && step < maxSteps)
        return patterns[(size_t) pat].var[var].steps[(size_t) voice][(size_t) step];
    return false;
}

void Sequencer::setAccent (int pat, int var, int step, bool on)
{
    if (pat >= 0 && pat < numPatterns && (var == 0 || var == 1) && step >= 0 && step < maxSteps)
        patterns[(size_t) pat].var[var].accent[(size_t) step] = on;
}

void Sequencer::setFlam (int pat, int var, int voice, int step, bool on)
{
    if (pat >= 0 && pat < numPatterns && (var == 0 || var == 1) && validVoice (voice) && step >= 0 && step < maxSteps)
        patterns[(size_t) pat].var[var].flam[(size_t) voice][(size_t) step] = on;
}

void Sequencer::setProbability (int pat, int var, int voice, int step, float p)
{
    if (pat >= 0 && pat < numPatterns && (var == 0 || var == 1) && validVoice (voice) && step >= 0 && step < maxSteps)
        patterns[(size_t) pat].var[var].prob[(size_t) voice][(size_t) step] = juce::jlimit (0.0f, 1.0f, p);
}

void Sequencer::setLength (int pat, int var, int length)
{
    if (pat >= 0 && pat < numPatterns && (var == 0 || var == 1))
        patterns[(size_t) pat].var[var].length = juce::jlimit (1, maxSteps, length);
}

//== State =====================================================================
static juce::String boolRowToString (const std::array<bool, Sequencer::maxSteps>& row)
{
    juce::String s;
    for (bool b : row) s += (b ? '1' : '0');
    return s;
}

static void stringToBoolRow (const juce::String& s, std::array<bool, Sequencer::maxSteps>& row)
{
    for (int i = 0; i < Sequencer::maxSteps && i < s.length(); ++i)
        row[(size_t) i] = (s[i] == '1');
}

juce::ValueTree Sequencer::toValueTree() const
{
    juce::ValueTree state ("SEQUENCER");
    state.setProperty ("currentPattern", currentPattern, nullptr);
    state.setProperty ("playMode", (int) playMode, nullptr);
    state.setProperty ("chainEnabled", chainEnabled, nullptr);
    state.setProperty ("tempo", internalBpm, nullptr);
    state.setProperty ("swing", swing, nullptr);
    state.setProperty ("running", running, nullptr);
    state.setProperty ("probEnabled", probEnabled, nullptr);

    juce::String chainStr, muteStr, soloStr;
    for (int c : chain)        chainStr += juce::String (c) + ",";
    for (bool m : muted)       muteStr  += (m ? '1' : '0');
    for (bool so : soloed)     soloStr  += (so ? '1' : '0');
    state.setProperty ("chain", chainStr, nullptr);
    state.setProperty ("mute", muteStr, nullptr);
    state.setProperty ("solo", soloStr, nullptr);

    for (int p = 0; p < numPatterns; ++p)
    {
        juce::ValueTree pt ("PATTERN");
        pt.setProperty ("index", p, nullptr);
        for (int var = 0; var < 2; ++var)
        {
            const auto& v = patterns[(size_t) p].var[var];
            juce::ValueTree vt ("VAR");
            vt.setProperty ("v", var, nullptr);
            vt.setProperty ("length", v.length, nullptr);
            vt.setProperty ("accent", boolRowToString (v.accent), nullptr);
            for (int vo = 0; vo < numVoices; ++vo)
            {
                vt.setProperty ("steps" + juce::String (vo), boolRowToString (v.steps[(size_t) vo]), nullptr);
                vt.setProperty ("flam"  + juce::String (vo), boolRowToString (v.flam[(size_t) vo]),  nullptr);
            }
            pt.addChild (vt, -1, nullptr);
        }
        state.addChild (pt, -1, nullptr);
    }
    return state;
}

void Sequencer::fromValueTree (const juce::ValueTree& state)
{
    if (! state.hasType ("SEQUENCER"))
        return;

    currentPattern = juce::jlimit (0, numPatterns - 1, (int) state.getProperty ("currentPattern", 0));
    playMode       = (PlayMode) (int) state.getProperty ("playMode", 0);
    chainEnabled   = (bool) state.getProperty ("chainEnabled", false);
    internalBpm    = (double) state.getProperty ("tempo", 120.0);
    swing          = (float) (double) state.getProperty ("swing", 0.0);
    running        = (bool) state.getProperty ("running", false);
    probEnabled    = (bool) state.getProperty ("probEnabled", false);

    chain.clear();
    auto tokens = juce::StringArray::fromTokens (state.getProperty ("chain").toString(), ",", "");
    for (auto& tk : tokens) if (tk.isNotEmpty()) chain.push_back (tk.getIntValue());

    const juce::String muteStr = state.getProperty ("mute").toString();
    const juce::String soloStr = state.getProperty ("solo").toString();
    for (int i = 0; i < numVoices; ++i)
    {
        muted[(size_t) i]  = (i < muteStr.length() && muteStr[i] == '1');
        soloed[(size_t) i] = (i < soloStr.length() && soloStr[i] == '1');
    }

    for (int c = 0; c < state.getNumChildren(); ++c)
    {
        const auto pt = state.getChild (c);
        if (! pt.hasType ("PATTERN")) continue;
        const int p = juce::jlimit (0, numPatterns - 1, (int) pt.getProperty ("index", 0));
        for (int ci = 0; ci < pt.getNumChildren(); ++ci)
        {
            const auto vt = pt.getChild (ci);
            if (! vt.hasType ("VAR")) continue;
            const int var = (int) vt.getProperty ("v", 0);
            if (var != 0 && var != 1) continue;
            auto& v = patterns[(size_t) p].var[var];
            v.length = juce::jlimit (1, maxSteps, (int) vt.getProperty ("length", 16));
            stringToBoolRow (vt.getProperty ("accent").toString(), v.accent);
            for (int vo = 0; vo < numVoices; ++vo)
            {
                stringToBoolRow (vt.getProperty ("steps" + juce::String (vo)).toString(), v.steps[(size_t) vo]);
                stringToBoolRow (vt.getProperty ("flam"  + juce::String (vo)).toString(), v.flam[(size_t) vo]);
            }
        }
    }
}
}
