#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "PluginProcessor.h"

//==============================================================================
// M0 editor: a minimal, resizable window. Replaced by the PERFORM/EDIT views
// (808 panel, step sequencer) in M6.
//==============================================================================
class TR808AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit TR808AudioProcessorEditor (TR808AudioProcessor&);
    ~TR808AudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TR808AudioProcessor& processorRef;

    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TR808AudioProcessorEditor)
};
