#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace params
{
    // Builds the full APVTS parameter layout. Kept free of any UI / processor
    // state so it can be unit-tested in isolation.
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
}
