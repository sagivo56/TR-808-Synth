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

        // visible beat / bar grouping (4/4 or 3/4): a line at each beat, a bolder
        // one at each bar boundary.
        const float gh = (float) (numVoices + 1) * rh;
        for (int s = stepsPerBeat; s < len; s += stepsPerBeat)
        {
            const bool bar = (s % (stepsPerBeat * beatsPerBar)) == 0;
            const float x = area.getX() + s * cw;
            g.setColour (bar ? Colors::orange.withAlpha (0.9f) : Colors::cream.withAlpha (0.32f));
            g.fillRect (x - (bar ? 1.0f : 0.5f), (float) area.getY(), bar ? 2.0f : 1.0f, gh);
        }
    }
    else // authentic 808-style: instrument selector + edit-layer + step keys
    {
        juce::ignoreUnused (cell);
        const int  numInst = numVoices + 1;                // 16 voices + ACCENT
        const bool accSel  = (selectedVoice == accentIndex);

        // instrument selector
        auto selRow = area.removeFromTop (juce::jmax (22, area.getHeight() / 6));
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

        area.removeFromTop (4);

        // edit-layer selector: STEP / FLAM / PROB
        auto layerRow = area.removeFromTop (juce::jmax (16, area.getHeight() / 7));
        const char* lyNames[3] = { "STEP", "FLAM", "PROB" };
        const float lw = juce::jmin (66.0f, layerRow.getWidth() / 6.0f);
        for (int i = 0; i < 3; ++i)
        {
            juce::Rectangle<float> r (layerRow.getX() + i * lw, (float) layerRow.getY(), lw - 2.0f, (float) layerRow.getHeight());
            const bool sel = ((int) layer == i);
            const bool disabled = accSel && i != 0;
            g.setColour (sel ? Colors::orange : (disabled ? Colors::background : Colors::panelLight));
            g.fillRoundedRectangle (r.reduced (1.0f), 3.0f);
            g.setColour (sel ? Colors::background : (disabled ? Colors::grayOff : Colors::text));
            g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
            g.drawText (lyNames[i], r, juce::Justification::centred);
        }

        area.removeFromTop (4);

        // step keys for the selected instrument, per layer
        const float cw = area.getWidth() / (float) len;
        const float bh = (float) juce::jmin (area.getHeight(), 80);
        for (int s = 0; s < len; ++s)
        {
            juce::Rectangle<float> r (area.getX() + s * cw, (float) area.getY(), cw, bh);
            const bool isDown = (s % 4 == 0);
            const juce::Colour offCol = isDown ? Colors::grayOff.brighter (0.18f) : Colors::grayOff;
            g.setColour (offCol);
            g.fillRoundedRectangle (r.reduced (2.0f), 3.0f);

            if (accSel)
            {
                if (seq.getAccent (pat, editVar, s)) { g.setColour (Colors::yellow); g.fillRoundedRectangle (r.reduced (2.0f), 3.0f); }
            }
            else if (layer == Layer::step)
            {
                if (seq.getStep (pat, editVar, selectedVoice, s)) { g.setColour (isDown ? Colors::red : Colors::orange); g.fillRoundedRectangle (r.reduced (2.0f), 3.0f); }
            }
            else if (layer == Layer::flam)
            {
                if (seq.getFlam (pat, editVar, selectedVoice, s)) { g.setColour (Colors::cream); g.fillRoundedRectangle (r.reduced (2.0f), 3.0f); }
            }
            else // prob: fill height = probability
            {
                const float p = seq.getProbability (pat, editVar, selectedVoice, s);
                auto fill = r.reduced (2.0f);
                fill = fill.removeFromBottom (fill.getHeight() * juce::jlimit (0.0f, 1.0f, p));
                g.setColour (Colors::orange);
                g.fillRoundedRectangle (fill, 3.0f);
            }

            if (s == playStep) { g.setColour (Colors::white); g.drawRoundedRectangle (r.reduced (2.0f), 3.0f, 2.0f); }
        }

        // bar boundary lines over the step keys (visible meter grouping)
        for (int s = stepsPerBeat; s < len; s += stepsPerBeat)
        {
            const bool bar = (s % (stepsPerBeat * beatsPerBar)) == 0;
            const float x = area.getX() + s * cw;
            g.setColour (bar ? Colors::orange.withAlpha (0.9f) : Colors::cream.withAlpha (0.28f));
            g.fillRect (x - (bar ? 1.0f : 0.5f), (float) area.getY(), bar ? 2.0f : 1.0f, bh);
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
        auto labels = area.removeFromLeft (kLabelW);
        const int   rowsAll = numVoices + 1;
        const float rhAll   = area.getHeight() / (float) rowsAll;
        if (labels.contains (e.getPosition()))            // click an instrument name -> preview
        {
            const int row = (int) ((e.y - area.getY()) / rhAll);
            if (row >= 0 && row < numVoices && onPreview) onPreview (row);
            return;
        }
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
        const int  numInst = numVoices + 1;
        const bool accSel  = (selectedVoice == accentIndex);

        auto selRow = area.removeFromTop (juce::jmax (22, area.getHeight() / 6));
        if (selRow.contains (e.getPosition()))
        {
            const float iw = selRow.getWidth() / (float) numInst;
            const int i = (int) ((e.x - selRow.getX()) / iw);
            if (i >= 0 && i < numInst)
            {
                selectedVoice = i;
                if (onSelect) onSelect (i);
                if (i < numVoices && onPreview) onPreview (i);   // also audition it
            }
            repaint();
            return;
        }

        area.removeFromTop (4);
        auto layerRow = area.removeFromTop (juce::jmax (16, area.getHeight() / 7));
        if (layerRow.contains (e.getPosition()))
        {
            const float lw = juce::jmin (66.0f, layerRow.getWidth() / 6.0f);
            const int i = (int) ((e.x - layerRow.getX()) / lw);
            if (i >= 0 && i < 3 && ! (accSel && i != 0)) layer = (Layer) i;
            repaint();
            return;
        }

        area.removeFromTop (4);
        const float cw = area.getWidth() / (float) len;
        const float bh = (float) juce::jmin (area.getHeight(), 80);
        const int s = (int) ((e.x - area.getX()) / cw);
        if (s >= 0 && s < len && e.y <= area.getY() + bh)
        {
            if (accSel || layer == Layer::step)
            {
                if (accSel) seq.setAccent (pat, editVar, s, ! seq.getAccent (pat, editVar, s));
                else        seq.setStep (pat, editVar, selectedVoice, s, ! seq.getStep (pat, editVar, selectedVoice, s));
            }
            else if (layer == Layer::flam)
            {
                seq.setFlam (pat, editVar, selectedVoice, s, ! seq.getFlam (pat, editVar, selectedVoice, s));
            }
            else // prob: cycle 1.0 -> 0.75 -> 0.5 -> 0.25 -> 1.0
            {
                const float p = seq.getProbability (pat, editVar, selectedVoice, s);
                const float next = (p > 0.99f) ? 0.75f : (p > 0.7f) ? 0.5f : (p > 0.45f) ? 0.25f : 1.0f;
                seq.setProbability (pat, editVar, selectedVoice, s, next);
            }
        }
    }

    repaint();
}
}
