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
    else // authentic 808-style: instrument selector (incl. ACCENT) + step keys
    {
        juce::ignoreUnused (cell);
        const int numInst = numVoices + 1;                 // 16 voices + ACCENT

        // instrument selector
        auto selRow = area.removeFromTop (juce::jmax (24, area.getHeight() / 5));
        const float iw = selRow.getWidth() / (float) numInst;
        for (int i = 0; i < numInst; ++i)
        {
            juce::Rectangle<float> r (selRow.getX() + i * iw, (float) selRow.getY(), iw, (float) selRow.getHeight());
            const bool sel = (i == selectedVoice);
            g.setColour (sel ? Colors::orange : Colors::panelLight);
            g.fillRoundedRectangle (r.reduced (1.5f), 3.0f);
            g.setColour (sel ? Colors::background : Colors::text);
            g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
            g.drawText (i == accentIndex ? "ACC" : juce::String (voiceSpecs()[(size_t) i].prefix).toUpperCase(),
                        r, juce::Justification::centred);
        }

        area.removeFromTop (6);

        // step keys for the selected instrument (or accent), 808 grouping colours
        const bool accSel = (selectedVoice == accentIndex);
        const float cw = area.getWidth() / (float) len;
        const float bh = (float) juce::jmin (area.getHeight(), 90);
        for (int s = 0; s < len; ++s)
        {
            juce::Rectangle<float> r (area.getX() + s * cw, (float) area.getY(), cw, bh);
            const bool on = accSel ? seq.getAccent (pat, editVar, s)
                                   : seq.getStep (pat, editVar, selectedVoice, s);
            const juce::Colour offCol = (s % 4 == 0) ? Colors::grayOff.brighter (0.18f) : Colors::grayOff;
            const juce::Colour onCol  = accSel ? Colors::yellow : ((s % 4 == 0) ? Colors::red : Colors::orange);
            g.setColour (on ? onCol : offCol);
            g.fillRoundedRectangle (r.reduced (2.0f), 3.0f);
            if (s == playStep) { g.setColour (Colors::white); g.drawRoundedRectangle (r.reduced (2.0f), 3.0f, 2.0f); }
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
        const int numInst = numVoices + 1;
        auto selRow = area.removeFromTop (juce::jmax (24, area.getHeight() / 5));
        if (selRow.contains (e.getPosition()))
        {
            const float iw = selRow.getWidth() / (float) numInst;
            const int i = (int) ((e.x - selRow.getX()) / iw);
            if (i >= 0 && i < numInst) { selectedVoice = i; if (onSelect) onSelect (i); }
            repaint();
            return;
        }
        area.removeFromTop (6);
        const float cw = area.getWidth() / (float) len;
        const float bh = (float) juce::jmin (area.getHeight(), 90);
        const int s = (int) ((e.x - area.getX()) / cw);
        if (s >= 0 && s < len && e.y <= area.getY() + bh)
        {
            if (selectedVoice == accentIndex) seq.setAccent (pat, editVar, s, ! seq.getAccent (pat, editVar, s));
            else                              seq.setStep (pat, editVar, selectedVoice, s, ! seq.getStep (pat, editVar, selectedVoice, s));
        }
    }

    repaint();
}
}
