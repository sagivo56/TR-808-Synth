#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Sequencer.h"

namespace tr808
{
//==============================================================================
// Kit presets (the synthesis sound: macro + deep params + master drive) and
// Pattern presets (the sequencer) are independent. Factory banks are built in;
// user presets are saved/loaded as XML.
//==============================================================================
class PresetManager
{
public:
    static juce::StringArray kitNames();
    static juce::StringArray patternNames();

    static void applyFactoryKit (juce::AudioProcessorValueTreeState&, int index);
    static void applyFactoryPattern (Sequencer&, int index);

    static void saveKit  (juce::AudioProcessorValueTreeState&, const juce::File&);
    static bool loadKit  (juce::AudioProcessorValueTreeState&, const juce::File&);
    static void savePattern (const Sequencer&, const juce::File&);
    static bool loadPattern (Sequencer&, const juce::File&);

    static juce::File presetsDir();   // %APPDATA%/TR-808 Synth/Presets
};
}
