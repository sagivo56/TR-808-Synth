#include "PluginEditor.h"

TR808AudioProcessorEditor::TR808AudioProcessorEditor (TR808AudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    titleLabel.setText ("TR-808 Synth", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::FontOptions (28.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xffff7a18)); // 808 orange
    addAndMakeVisible (titleLabel);

    setResizable (true, true);
    setResizeLimits (320, 200, 1600, 1000);
    setSize (480, 300);
}

void TR808AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a1a));

    g.setColour (juce::Colour (0xff8a8a8a));
    g.setFont (juce::FontOptions (13.0f));
    g.drawText ("M0 scaffold \xE2\x80\x94 silent output",
                getLocalBounds().removeFromBottom (40),
                juce::Justification::centred);
}

void TR808AudioProcessorEditor::resized()
{
    titleLabel.setBounds (getLocalBounds().reduced (20).removeFromTop (60));
}
