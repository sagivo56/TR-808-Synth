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
    const int len = juce::jlimit (1, Sequencer::maxSteps, seq.getLength (pat, editVar));
    auto area = gridArea();

    auto cell = [&] (juce::Rectangle<float> r, bool on, bool accent, bool playing)
    {
        g.setColour (on ? (accent ? Colors::yellow : Colors::orange) : Colors::grayOff);
        g.fillRoundedRectangle (r.reduced (1.0f), 2.0f);
        if (playing) { g.setColour (Colors::white); g.drawRoundedRectangle (r.reduced (1.0f), 2.0f, 1.5f); }
    };

    if (mode == Mode::grid)
    {
        auto labels = area.removeFromLeft (kLabelW);
        const int   rows = numVoices + 1;                 // voices + accent lane
        const float cw = area.getWidth() / (float) len;
        const float rh = area.getHeight() / (float) rows;

        for (int v = 0; v < numVoices; ++v)
        {
            g.setColour (Colors::text);
            g.setFont (juce::FontOptions (10.0f));
            g.drawText (juce::String (voiceSpecs()[(size_t) v].prefix).toUpperCase(),
                        labels.getX(), (int) (area.getY() + v * rh), kLabelW - 4, (int) rh,
                        juce::Justification::centredRight);
            for (int s = 0; s < len; ++s)
                cell ({ area.getX() + s * cw, area.getY() + v * rh, cw, rh },
                      seq.getStep (pat, editVar, v, s), seq.getAccent (pat, editVar, s), s == playStep);
        }

        // accent lane
        g.setColour (Colors::yellow);
        g.setFont (juce::FontOptions (10.0f));
        g.drawText ("ACC", labels.getX(), (int) (area.getY() + numVoices * rh), kLabelW - 4, (int) rh,
                    juce::Justification::centredRight);
        for (int s = 0; s < len; ++s)
            cell ({ area.getX() + s * cw, area.getY() + numVoices * rh, cw, rh },
                  seq.getAccent (pat, editVar, s), true, s == playStep);
    }
    else // authentic: selected voice + accent row
    {
        g.setColour (Colors::orange);
        g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
        g.drawText (juce::String (voiceSpecs()[(size_t) selectedVoice].name),
                    area.removeFromTop (22), juce::Justification::centredLeft);

        const float cw = area.getWidth() / (float) len;
        auto stepRow   = area.removeFromTop (area.getHeight() / 2).reduced (0, 4);
        auto accentRow = area.reduced (0, 4);

        for (int s = 0; s < len; ++s)
        {
            cell ({ stepRow.getX() + s * cw, (float) stepRow.getY(), cw, (float) stepRow.getHeight() },
                  seq.getStep (pat, editVar, selectedVoice, s), false, s == playStep);
            cell ({ accentRow.getX() + s * cw, (float) accentRow.getY(), cw, (float) accentRow.getHeight() },
                  seq.getAccent (pat, editVar, s), true, s == playStep);
        }
    }
}

void StepSequencerView::mouseDown (const juce::MouseEvent& e)
{
    const int pat = seq.getCurrentPattern();
    const int len = juce::jlimit (1, Sequencer::maxSteps, seq.getLength (pat, editVar));
    auto area = gridArea();

    if (mode == Mode::grid)
    {
        area.removeFromLeft (kLabelW);
        if (! area.contains (e.getPosition())) return;
        const int   rows = numVoices + 1;
        const float cw = area.getWidth() / (float) len;
        const float rh = area.getHeight() / (float) rows;
        const int s   = (int) ((e.x - area.getX()) / cw);
        const int row = (int) ((e.y - area.getY()) / rh);
        if (s < 0 || s >= len) return;
        if (row >= 0 && row < numVoices) seq.setStep (pat, editVar, row, s, ! seq.getStep (pat, editVar, row, s));
        else if (row == numVoices)       seq.setAccent (pat, editVar, s, ! seq.getAccent (pat, editVar, s));
    }
    else
    {
        area.removeFromTop (22);
        const float cw = area.getWidth() / (float) len;
        auto stepRow   = area.removeFromTop (area.getHeight() / 2).reduced (0, 4);
        auto accentRow = area.reduced (0, 4);
        const int s = (int) ((e.x - area.getX()) / cw);
        if (s < 0 || s >= len) return;
        if (stepRow.contains (e.getPosition()))        seq.setStep (pat, editVar, selectedVoice, s, ! seq.getStep (pat, editVar, selectedVoice, s));
        else if (accentRow.contains (e.getPosition())) seq.setAccent (pat, editVar, s, ! seq.getAccent (pat, editVar, s));
    }

    repaint();
}
}
