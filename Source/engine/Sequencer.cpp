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
    {
        uiStep.store (-1);
        return;
    }

    const double ppqPerSample = (bpm / 60.0) / sr;
    const double ppqStart = t.hostPlaying ? t.ppqPosition : internalPpq;
    const double ppqEnd   = ppqStart + (double) t.numSamples * ppqPerSample;

    // Step duration in quarter notes (0.25 = 16ths; smaller = triplets).
    double stepQ = (double) patterns[(size_t) currentPattern].var[0].stepDiv;
    if (stepQ < 1.0e-4) stepQ = 0.25;
    const double flamQ = (double) flamMs * 0.001 * (bpm / 60.0);
    const int    refLen = juce::jmax (1, patterns[(size_t) currentPattern].var[0].length);

    {   // expose the current step for the UI playhead
        const long long cur = (long long) std::floor (ppqStart / stepQ);
        uiStep.store ((int) (((cur % refLen) + refLen) % refLen));
    }

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
                         : (playMode == PlayMode::c) ? 2
                         : (playMode == PlayMode::d) ? 3
                         : (int) (((loopIndex % numVars) + numVars) % numVars);     // cycle A->B->C->D
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

        // melodic bass note on this step
        const int bn = v.bassNote[(size_t) stepInLoop];
        if (bn >= 0 && mainInBlock)
        {
            const int off = juce::jlimit (0, t.numSamples - 1, (int) ((mainPpq - ppqStart) / ppqPerSample));
            const float hz = 440.0f * std::pow (2.0f, (float) (bn - 69) / 12.0f);
            out.push_back ({ off, VoiceManager::bassVoiceIndex, 0.85f, v.accent[(size_t) stepInLoop], hz });
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
    if (pat >= 0 && pat < numPatterns && validVar (var) && validVoice (voice) && step >= 0 && step < maxSteps)
        patterns[(size_t) pat].var[var].steps[(size_t) voice][(size_t) step] = on;
}

bool Sequencer::getStep (int pat, int var, int voice, int step) const
{
    if (pat >= 0 && pat < numPatterns && validVar (var) && validVoice (voice) && step >= 0 && step < maxSteps)
        return patterns[(size_t) pat].var[var].steps[(size_t) voice][(size_t) step];
    return false;
}

void Sequencer::setAccent (int pat, int var, int step, bool on)
{
    if (pat >= 0 && pat < numPatterns && validVar (var) && step >= 0 && step < maxSteps)
        patterns[(size_t) pat].var[var].accent[(size_t) step] = on;
}

void Sequencer::setFlam (int pat, int var, int voice, int step, bool on)
{
    if (pat >= 0 && pat < numPatterns && validVar (var) && validVoice (voice) && step >= 0 && step < maxSteps)
        patterns[(size_t) pat].var[var].flam[(size_t) voice][(size_t) step] = on;
}

void Sequencer::setProbability (int pat, int var, int voice, int step, float p)
{
    if (pat >= 0 && pat < numPatterns && validVar (var) && validVoice (voice) && step >= 0 && step < maxSteps)
        patterns[(size_t) pat].var[var].prob[(size_t) voice][(size_t) step] = juce::jlimit (0.0f, 1.0f, p);
}

void Sequencer::setLength (int pat, int var, int length)
{
    if (pat >= 0 && pat < numPatterns && validVar (var))
        patterns[(size_t) pat].var[var].length = juce::jlimit (1, maxSteps, length);
}

int Sequencer::getLength (int pat, int var) const
{
    if (pat >= 0 && pat < numPatterns && validVar (var))
        return patterns[(size_t) pat].var[var].length;
    return maxSteps;
}

void Sequencer::clearPattern (int pat)
{
    if (pat < 0 || pat >= numPatterns) return;
    for (int var = 0; var < numVars; ++var)
    {
        auto& v = patterns[(size_t) pat].var[var];
        for (auto& row : v.steps) row.fill (false);
        for (auto& row : v.flam)  row.fill (false);
        for (auto& row : v.prob)  row.fill (1.0f);
        v.accent.fill (false);
        v.bassNote.fill (-1);
    }
}

void Sequencer::setBassNote (int pat, int var, int step, int midiNote)
{
    if (pat >= 0 && pat < numPatterns && validVar (var) && step >= 0 && step < maxSteps)
        patterns[(size_t) pat].var[var].bassNote[(size_t) step] = (midiNote < 0 ? -1 : midiNote);
}

int Sequencer::getBassNote (int pat, int var, int step) const
{
    if (pat >= 0 && pat < numPatterns && validVar (var) && step >= 0 && step < maxSteps)
        return patterns[(size_t) pat].var[var].bassNote[(size_t) step];
    return -1;
}

void Sequencer::setStepDiv (int pat, int var, float div)
{
    if (pat >= 0 && pat < numPatterns && validVar (var))
        patterns[(size_t) pat].var[var].stepDiv = juce::jlimit (0.05f, 1.0f, div);
}

float Sequencer::getStepDiv (int pat, int var) const
{
    if (pat >= 0 && pat < numPatterns && validVar (var))
        return patterns[(size_t) pat].var[var].stepDiv;
    return 0.25f;
}

bool Sequencer::getAccent (int pat, int var, int step) const
{
    if (pat >= 0 && pat < numPatterns && validVar (var) && step >= 0 && step < maxSteps)
        return patterns[(size_t) pat].var[var].accent[(size_t) step];
    return false;
}

bool Sequencer::getFlam (int pat, int var, int voice, int step) const
{
    if (pat >= 0 && pat < numPatterns && validVar (var) && voice >= 0 && voice < numVoices && step >= 0 && step < maxSteps)
        return patterns[(size_t) pat].var[var].flam[(size_t) voice][(size_t) step];
    return false;
}

float Sequencer::getProbability (int pat, int var, int voice, int step) const
{
    if (pat >= 0 && pat < numPatterns && validVar (var) && voice >= 0 && voice < numVoices && step >= 0 && step < maxSteps)
        return patterns[(size_t) pat].var[var].prob[(size_t) voice][(size_t) step];
    return 1.0f;
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
    state.setProperty ("bassRoot", bassRoot, nullptr);
    state.setProperty ("bassScale", bassScale, nullptr);
    state.setProperty ("bassGate", bassGate, nullptr);

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
        for (int var = 0; var < numVars; ++var)
        {
            const auto& v = patterns[(size_t) p].var[var];
            juce::ValueTree vt ("VAR");
            vt.setProperty ("v", var, nullptr);
            vt.setProperty ("length", v.length, nullptr);
            vt.setProperty ("stepdiv", v.stepDiv, nullptr);
            vt.setProperty ("accent", boolRowToString (v.accent), nullptr);
            for (int vo = 0; vo < numVoices; ++vo)
            {
                vt.setProperty ("steps" + juce::String (vo), boolRowToString (v.steps[(size_t) vo]), nullptr);
                vt.setProperty ("flam"  + juce::String (vo), boolRowToString (v.flam[(size_t) vo]),  nullptr);
                // probability quantised to 4 levels (1.0/0.75/0.5/0.25) as one char per step
                juce::String pr;
                for (float p : v.prob[(size_t) vo])
                    pr += (juce::juce_wchar) ('0' + juce::jlimit (0, 3, (int) std::lround ((1.0f - p) / 0.25f)));
                vt.setProperty ("prob" + juce::String (vo), pr, nullptr);
            }
            juce::String bassStr;
            for (int n : v.bassNote) bassStr += juce::String (n) + ",";
            vt.setProperty ("bass", bassStr, nullptr);
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
    bassRoot       = juce::jlimit (0, 11, (int) state.getProperty ("bassRoot", 0));
    bassScale      = juce::jlimit (0, 4,  (int) state.getProperty ("bassScale", 0));
    bassGate       = (float) (double) state.getProperty ("bassGate", 0.5);

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
            if (! validVar (var)) continue;
            auto& v = patterns[(size_t) p].var[var];
            v.length  = juce::jlimit (1, maxSteps, (int) vt.getProperty ("length", 16));
            v.stepDiv = juce::jlimit (0.05f, 1.0f, (float) (double) vt.getProperty ("stepdiv", 0.25));
            stringToBoolRow (vt.getProperty ("accent").toString(), v.accent);
            for (int vo = 0; vo < numVoices; ++vo)
            {
                stringToBoolRow (vt.getProperty ("steps" + juce::String (vo)).toString(), v.steps[(size_t) vo]);
                stringToBoolRow (vt.getProperty ("flam"  + juce::String (vo)).toString(), v.flam[(size_t) vo]);
                const auto pr = vt.getProperty ("prob" + juce::String (vo)).toString();
                for (int s = 0; s < maxSteps && s < pr.length(); ++s)
                    v.prob[(size_t) vo][(size_t) s] = 1.0f - (float) juce::jlimit (0, 3, (int) pr[s] - (int) '0') * 0.25f;
            }
            v.bassNote.fill (-1);
            const auto bt = juce::StringArray::fromTokens (vt.getProperty ("bass").toString(), ",", "");
            for (int s = 0; s < maxSteps && s < bt.size(); ++s)
                if (bt[s].isNotEmpty())
                    v.bassNote[(size_t) s] = juce::jlimit (-1, 127, bt[s].getIntValue());
        }
    }
}
}
