#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace tr808::ui
{
// Classic TR-808 palette.
namespace Colors
{
    const juce::Colour background { 0xff1e1e1e };
    const juce::Colour panel      { 0xff2c2c2c };
    const juce::Colour panelLight { 0xff3a3a3a };
    const juce::Colour orange     { 0xfff07a18 };   // accent / step on
    const juce::Colour red        { 0xffd8392e };
    const juce::Colour yellow     { 0xfff2c14e };
    const juce::Colour cream      { 0xffe9e2d0 };
    const juce::Colour white      { 0xffeeeeee };
    const juce::Colour grayOff     { 0xff555555 };  // step off
    const juce::Colour text       { 0xffd8d8d8 };
}

class LookAndFeel808 : public juce::LookAndFeel_V4
{
public:
    LookAndFeel808()
    {
        setColour (juce::ResizableWindow::backgroundColourId, Colors::background);
        setColour (juce::Slider::textBoxTextColourId, Colors::text);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId, Colors::text);
        setColour (juce::TextButton::buttonColourId, Colors::panelLight);
        setColour (juce::TextButton::textColourOffId, Colors::text);
        setColour (juce::TextButton::textColourOnId, Colors::background);
        setColour (juce::ComboBox::backgroundColourId, Colors::panelLight);
        setColour (juce::ComboBox::textColourId, Colors::text);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float pos, float startAngle, float endAngle, juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
        const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto angle  = startAngle + pos * (endAngle - startAngle);

        g.setColour (Colors::cream);
        g.fillEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre));
        g.setColour (Colors::background);
        g.drawEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre), 1.5f);

        // pointer
        juce::Path p;
        p.addRoundedRectangle (-1.5f, -radius + 2.0f, 3.0f, radius * 0.55f, 1.5f);
        g.setColour (Colors::red);
        g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centre));
    }
};
}
