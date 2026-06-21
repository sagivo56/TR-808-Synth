#include "StepSequencerView.h"

namespace tr808::ui
{
StepSequencerView::StepSequencerView (Sequencer& sequencer) : seq (sequencer)
{
    startTimerHz (30);
}

StepSequencerView::~StepSequencerView() { stopTimer(); }

void StepSequencerView::timerCallback()
{
    const int d = seq.getDisplayStep();
    if (d != lastDisplay) { lastDisplay = d; repaint(); }
}

juce::Rectangle<int> StepSequencerView::gridArea() const
{
    return getLocalBounds().reduced (4);
}

void StepSequencerView::paint (juce::Graphics& g)
{
    g.setColour (Colors::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 4.0f);

    const int pat = seq.getCurrentPattern();
    const int playStep = seq.getDisplayStep();
    auto area = gridArea();

    auto cell = [&] (juce::Rectangle<float> r, bool on, bool accent, bool beyond, bool playing)
    {
        juce::Colour c = on ? (accent ? Colors::yellow : Colors::orange)
                            : (beyond ? Colors::background : Colors::grayOff);
        g.setColour (c);
        g.fillRoundedRectangle (r.reduced (1.5f), 2.0f);
        if (playing) { g.setColour (Colors::white); g.drawRoundedRectangle (r.reduced (1.5f), 2.0f, 1.5f); }
    };

    if (mode == Mode::grid)
    {
        auto labels = area.removeFromLeft (kLabelW);
        const float cw = area.getWidth() / 16.0f;
        const float rh = area.getHeight() / (float) numVoices;

        for (int v = 0; v < numVoices; ++v)
        {
            const int len = seq.getLength (pat, editVar);
            g.setColour (Colors::text);
            g.setFont (juce::FontOptions (11.0f));
            g.drawText (juce::String (voiceSpecs()[(size_t) v].prefix).toUpperCase(),
                        labels.getX(), (int) (area.getY() + v * rh), kLabelW - 4, (int) rh,
                        juce::Justification::centredRight);

            for (int s = 0; s < 16; ++s)
            {
                juce::Rectangle<float> r (area.getX() + s * cw, area.getY() + v * rh, cw, rh);
                cell (r, seq.getStep (pat, editVar, v, s), seq.getAccent (pat, editVar, s), s >= len, s == playStep);
            }
        }
    }
    else // authentic: selected voice row + accent row
    {
        g.setColour (Colors::orange);
        g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
        auto titleRow = area.removeFromTop (22);
        g.drawText (juce::String (voiceSpecs()[(size_t) selectedVoice].name),
                    titleRow, juce::Justification::centredLeft);

        const int len = seq.getLength (pat, editVar);
        const float cw = area.getWidth() / 16.0f;
        auto stepRow   = area.removeFromTop (area.getHeight() / 2).reduced (0, 4);
        auto accentRow = area.reduced (0, 4);

        for (int s = 0; s < 16; ++s)
        {
            juce::Rectangle<float> rs (stepRow.getX() + s * cw, (float) stepRow.getY(), cw, (float) stepRow.getHeight());
            cell (rs, seq.getStep (pat, editVar, selectedVoice, s), false, s >= len, s == playStep);
            juce::Rectangle<float> ra (accentRow.getX() + s * cw, (float) accentRow.getY(), cw, (float) accentRow.getHeight());
            cell (ra, seq.getAccent (pat, editVar, s), true, s >= len, s == playStep);
        }
        g.setColour (Colors::text);
        g.setFont (juce::FontOptions (10.0f));
        g.drawText ("ACCENT", accentRow.removeFromLeft (kLabelW).translated (-kLabelW - 2, 0), juce::Justification::centredRight);
    }
}

void StepSequencerView::mouseDown (const juce::MouseEvent& e)
{
    const int pat = seq.getCurrentPattern();
    auto area = gridArea();

    if (mode == Mode::grid)
    {
        area.removeFromLeft (kLabelW);
        if (! area.contains (e.getPosition())) return;
        const float cw = area.getWidth() / 16.0f;
        const float rh = area.getHeight() / (float) numVoices;
        const int s = (int) ((e.x - area.getX()) / cw);
        const int v = (int) ((e.y - area.getY()) / rh);
        if (s >= 0 && s < 16 && v >= 0 && v < numVoices)
            seq.setStep (pat, editVar, v, s, ! seq.getStep (pat, editVar, v, s));
    }
    else
    {
        area.removeFromTop (22);
        const float cw = area.getWidth() / 16.0f;
        auto stepRow   = area.removeFromTop (area.getHeight() / 2).reduced (0, 4);
        auto accentRow = area.reduced (0, 4);
        const int s = (int) ((e.x - area.getX()) / cw);
        if (s < 0 || s >= 16) return;
        if (stepRow.contains (e.getPosition()))
            seq.setStep (pat, editVar, selectedVoice, s, ! seq.getStep (pat, editVar, selectedVoice, s));
        else if (accentRow.contains (e.getPosition()))
            seq.setAccent (pat, editVar, s, ! seq.getAccent (pat, editVar, s));
    }

    repaint();
}
}
