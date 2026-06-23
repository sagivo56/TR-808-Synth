#include "ParameterLayout.h"
#include "ParameterIDs.h"
#include "../engine/VoiceDefs.h"
#include "../engine/DeepParams.h"

namespace params
{
using namespace tr808;

static void addMacro (juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                      int idx, const char* macro, const juce::String& label, float defaultValue)
{
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { macroId (idx, macro), 1 },
        label,
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f),
        defaultValue));
}

static void addDeep (juce::AudioProcessorValueTreeState::ParameterLayout& layout, const DeepDesc& d)
{
    juce::NormalisableRange<float> range (d.min, d.max);
    if (d.skewLow && d.def > d.min && d.def < d.max)
        range.setSkewForCentre (d.def);

    const juce::String label = juce::String (voiceSpecs()[(size_t) d.voice].name) + " " + d.label;
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { macroId (d.voice, d.suffix), 1 },
        label, range, d.def));
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Master gain.
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::masterGain, 1 },
        "Master Gain",
        juce::NormalisableRange<float> (-60.0f, 6.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    // Per-voice Macro params (the 808-panel knobs). Level is universal; the rest
    // are added only for the voices that expose them (see VoiceSpec).
    for (int i = 0; i < numVoices; ++i)
    {
        const auto& s = voiceSpecs()[(size_t) i];
        const juce::String n (s.name);

        addMacro (layout, i, "level", n + " Level", 0.8f);
        if (s.tone)   addMacro (layout, i, "tone",   n + " Tone",   0.5f);
        if (s.decay)  addMacro (layout, i, "decay",  n + " Decay",  0.5f);
        if (s.snappy) addMacro (layout, i, "snappy", n + " Snappy", 0.5f);
        if (s.tune)   addMacro (layout, i, "tune",   n + " Tune",   0.5f);
        addMacro (layout, i, "revsend", n + " Reverb Send", 0.0f);
        addMacro (layout, i, "dlysend", n + " Delay Send",  0.0f);
    }

    // Deep-edit params (per-stage synthesis controls, real units).
    for (const auto& d : deepParamDescs())
        addDeep (layout, d);

    // Mixer (M5): per-voice pan + mute/solo.
    for (int i = 0; i < numVoices; ++i)
    {
        const juce::String n (voiceSpecs()[(size_t) i].name);
        layout.add (std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { macroId (i, "pan"), 1 }, n + " Pan",
            juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
        layout.add (std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { macroId (i, "mute"), 1 }, n + " Mute", false));
        layout.add (std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { macroId (i, "solo"), 1 }, n + " Solo", false));
    }

    // Master drive + gentle limiter, and the stereo/multi-out toggle.
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::masterDrive, 1 }, "Master Drive",
        juce::NormalisableRange<float> (1.0f, 10.0f, 0.01f), 1.0f));
    layout.add (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { ParamIDs::multiOut, 1 }, "Multi-Out", false));

    // Global accent amount (how much accented steps boost).
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::accentLevel, 1 }, "Accent",
        juce::NormalisableRange<float> (1.0f, 3.0f, 0.01f), 1.5f));

    // Parallel FX: melodic-bass sends + the reverb and delay parameters.
    auto addF = [&] (const char* id, const juce::String& name, float lo, float hi, float def, const char* unit = "")
    {
        layout.add (std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float> (lo, hi), def,
            juce::AudioParameterFloatAttributes().withLabel (unit)));
    };
    addF (ParamIDs::bassLevel,    "BD Bass Level",  0.0f, 1.0f, 0.9f);
    addF (ParamIDs::bassTone,     "BD Bass Tone",   0.0f, 1.0f, 0.25f);
    addF (ParamIDs::bassDecay,    "BD Bass Decay",  50.0f, 1500.0f, 650.0f, "ms");
    addF (ParamIDs::bassPunch,    "BD Bass Punch",  1.0f, 3.0f, 2.0f);
    addF (ParamIDs::bassDrive,    "BD Bass Drive",  1.0f, 10.0f, 1.0f);
    addF (ParamIDs::bassRevSend,  "BD Bass Reverb Send", 0.0f, 1.0f, 0.0f);
    addF (ParamIDs::bassDlySend,  "BD Bass Delay Send",  0.0f, 1.0f, 0.0f);
    addF (ParamIDs::revPredelay,  "Reverb Predelay", 0.0f, 120.0f, 15.0f, "ms");
    addF (ParamIDs::revDecay,     "Reverb Decay",    0.2f, 12.0f,  2.2f,  "s");
    addF (ParamIDs::revBass,      "Reverb Bass",     0.25f, 2.5f,  1.4f);
    addF (ParamIDs::revCrossover, "Reverb Crossover",150.0f, 2000.0f, 500.0f, "Hz");
    addF (ParamIDs::revDamp,      "Reverb Damping",  0.0f, 1.0f, 0.35f);
    addF (ParamIDs::revDepth,     "Reverb Depth",    0.0f, 1.0f, 0.30f);
    addF (ParamIDs::revReturn,    "Reverb Return",   0.0f, 1.5f, 0.9f);
    addF (ParamIDs::dlyTime,      "Delay Time",      1.0f, 1000.0f, 350.0f, "ms");
    addF (ParamIDs::dlyFeedback,  "Delay Feedback",  0.0f, 0.95f, 0.35f);
    addF (ParamIDs::dlyTone,      "Delay Tone",      0.0f, 1.0f, 0.6f);
    addF (ParamIDs::dlyReturn,    "Delay Return",    0.0f, 1.5f, 0.9f);

    return layout;
}
}
