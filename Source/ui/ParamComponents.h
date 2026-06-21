#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel808.h"

namespace tr808::ui
{
// A labelled rotary bound to an APVTS parameter (attachment => never desyncs).
class ParamKnob : public juce::Component
{
public:
    ParamKnob (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& label)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 14);
        addAndMakeVisible (slider);

        name.setText (label, juce::dontSendNotification);
        name.setJustificationType (juce::Justification::centred);
        name.setFont (juce::FontOptions (11.0f));
        addAndMakeVisible (name);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        name.setBounds (r.removeFromBottom (14));
        slider.setBounds (r);
    }

private:
    juce::Slider slider;
    juce::Label  name;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// A toggle button bound to an APVTS bool parameter.
class ParamToggle : public juce::Component
{
public:
    ParamToggle (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                 const juce::String& label, juce::Colour onColour)
    {
        button.setClickingTogglesState (true);
        button.setButtonText (label);
        button.setColour (juce::TextButton::buttonOnColourId, onColour);
        addAndMakeVisible (button);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramId, button);
    }

    void resized() override { button.setBounds (getLocalBounds()); }

    juce::TextButton button;

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
};
}
