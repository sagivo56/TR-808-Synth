#pragma once

#include <juce_data_structures/juce_data_structures.h>

#include <array>
#include <vector>
#include <cstdint>
#include <atomic>

#include "VoiceDefs.h"
#include "VoiceManager.h"   // TriggerEvent

namespace tr808
{
//==============================================================================
// 808-style step sequencer.
//
// Steps are placed on the musical (PPQ) timeline: each step is a 16th note, so
// step boundaries are derived from the host's ppqPosition (sample-accurate, and
// correct across loops/seeks). When the host isn't playing, an internal clock
// (run flag + internal tempo) drives the same logic so it works standalone.
//
// Features: per-voice 16-step grid, A/B variations + AB play mode, a simple
// pattern chain (song), a global accent track, per-step flam and probability,
// per-voice mute/solo, and swing on the offbeat 16ths.
//
// Pattern data lives here and is serialised to a ValueTree (plugin state) — it
// is intentionally NOT exposed as automatable APVTS parameters.
//==============================================================================
class Sequencer
{
public:
    static constexpr int maxSteps    = 32;   // up to a double bar
    static constexpr int numPatterns = 8;
    static constexpr int numVars     = 4;    // variations A, B, C, D

    enum class PlayMode { a, b, c, d, cycle };   // single var, or cycle A->B->C->D

    struct TransportInfo
    {
        bool   hostPlaying = false;
        double bpm         = 120.0;
        double ppqPosition = 0.0;
        double sampleRate  = 44100.0;
        int    numSamples  = 0;
    };

    Sequencer();

    void prepare (double sampleRate);
    void reset();

    // Emits this block's trigger events (sorted by sample position) into 'out'.
    void process (const TransportInfo& t, std::vector<TriggerEvent>& out);

    //== Editing ===============================================================
    void setStep        (int pat, int var, int voice, int step, bool on);
    bool getStep        (int pat, int var, int voice, int step) const;
    void setAccent      (int pat, int var, int step, bool on);
    void setFlam        (int pat, int var, int voice, int step, bool on);
    void setProbability (int pat, int var, int voice, int step, float p);
    void setLength      (int pat, int var, int length);
    void clearPattern   (int pat);   // wipe both variations' steps/flam/prob/accent + bass (keeps length/div)
    void copyVariation  (int pat, int from, int to);   // duplicate one variation onto another (A->B etc.)

    //== Melodic 808 bass track =================================================
    void setBassNote (int pat, int var, int step, int midiNote);   // -1 = off
    int  getBassNote (int pat, int var, int step) const;
    void setBassScale (int root, int type) { bassRoot = juce::jlimit (0, 11, root); bassScale = juce::jlimit (0, 4, type); }
    int  getBassRoot()  const { return bassRoot; }
    int  getBassScale() const { return bassScale; }
    void  setBassGate (float g) { bassGate = juce::jlimit (0.0f, 1.0f, g); }
    float getBassGate() const   { return bassGate; }
    void setStepDiv     (int pat, int var, float quartersPerStep);   // 0.25 = 1/16, ~0.1667 = 1/16 triplet
    float getStepDiv    (int pat, int var) const;

    void setMute (int voice, bool m) { if (validVoice (voice)) muted[(size_t) voice]  = m; }
    void setSolo (int voice, bool s) { if (validVoice (voice)) soloed[(size_t) voice] = s; }

    void setTempo  (double bpm)   { internalBpm = juce::jlimit (20.0, 300.0, bpm); }
    void setSwing  (float amount) { swing = juce::jlimit (0.0f, 0.75f, amount); }
    void setPlayMode (PlayMode m) { playMode = m; }
    void setRunning  (bool r)     { running = r; if (! r) internalPpq = 0.0; }
    bool isRunning() const        { return running; }
    void setCurrentPattern (int p){ if (p >= 0 && p < numPatterns) currentPattern = p; }
    void setChain (const std::vector<int>& c) { chain = c; }
    void setChainEnabled (bool e) { chainEnabled = e; }
    void setProbabilityEnabled (bool e) { probEnabled = e; }

    //== UI read-back ==========================================================
    int      getDisplayStep() const { return uiStep.load(); }   // step under the playhead (-1 if stopped)
    double   getTempo() const       { return internalBpm; }
    float    getSwing() const       { return swing; }
    PlayMode getPlayMode() const    { return playMode; }
    int      getCurrentPattern() const { return currentPattern; }
    int      getLength (int pat, int var) const;
    bool     getAccent (int pat, int var, int step) const;
    bool     getFlam (int pat, int var, int voice, int step) const;
    float    getProbability (int pat, int var, int voice, int step) const;
    bool     isChainEnabled() const { return chainEnabled; }
    const std::vector<int>& getChain() const { return chain; }

    //== State =================================================================
    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree&);

private:
    struct Variation
    {
        int   length  = 16;
        float stepDiv = 0.25f;   // quarter-notes per step (0.25 = 16ths, 1/6 = 16th triplets)
        std::array<std::array<bool, maxSteps>, numVoices>  steps {};
        std::array<std::array<bool, maxSteps>, numVoices>  flam {};
        std::array<std::array<float, maxSteps>, numVoices> prob;
        std::array<bool, maxSteps> accent {};
        std::array<int,  maxSteps> bassNote;   // melodic bass note per step (-1 = off)

        Variation() { for (auto& row : prob) row.fill (1.0f); bassNote.fill (-1); }
    };

    struct Pattern { Variation var[numVars]; };   // A, B, C, D

    static bool validVoice (int v) { return v >= 0 && v < numVoices; }
    static bool validVar   (int v) { return v >= 0 && v < numVars; }
    bool voicePlayable (int voice) const;
    static float hashUnit (long long step, int voice);

    std::array<Pattern, numPatterns> patterns;
    int      currentPattern = 0;
    PlayMode playMode = PlayMode::a;

    std::vector<int> chain;
    bool chainEnabled = false;

    std::array<bool, numVoices> muted  {};
    std::array<bool, numVoices> soloed {};

    double internalBpm = 120.0;
    float  swing = 0.0f;
    bool   running = false;

    int    bassRoot  = 0;     // bass scale root (0 = C)
    int    bassScale = 0;     // 0 chromatic, 1 major, 2 minor, 3 maj-pent, 4 min-pent
    float  bassGate  = 0.5f;  // bass note length (0..1 -> ring)
    bool   probEnabled = true;    // harmless at the default 1.0 per step; active once lowered

    double sampleRate = 44100.0;
    double internalPpq = 0.0;
    float  flamMs = 28.0f;

    std::atomic<int> uiStep { -1 };   // current step for the UI playhead
};
}
