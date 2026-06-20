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
    }

    // Deep-edit params (per-stage synthesis controls, real units).
    for (const auto& d : deepParamDescs())
        addDeep (layout, d);

    return layout;
}
}
