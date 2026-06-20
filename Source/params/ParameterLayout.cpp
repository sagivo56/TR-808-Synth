#include "ParameterLayout.h"
#include "ParameterIDs.h"

namespace params
{
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // M0: a single master-gain parameter so the APVTS skeleton — host
    // automation plus state save/load — is exercisable end to end. The 16
    // voices, deep-edit params and sequencer parameters arrive from M2 on.
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { ParamIDs::masterGain, 1 },
        "Master Gain",
        juce::NormalisableRange<float> (-60.0f, 6.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    return layout;
}
}
