#include "StepSequencerView.h"

#include <cmath>
#include <vector>

namespace tr808::ui
{
namespace
{
    const char* kScaleNames[5] = { "CHROM", "MAJOR", "MINOR", "MAJ PENT", "MIN PENT" };
    const float kGateVals[4]   = { 0.30f, 0.50f, 0.72f, 0.95f };
    const char* kGateNames[4]  = { "SHORT", "MED", "LONG", "FULL" };

    // MIDI notes of the scale over two octaves, low -> high (C1 base + root).
    std::vector<int> bassScaleNotes (int root, int type)
    {
        static const std::vector<std::vector<int>> degs = {
            { 0,1,2,3,4,5,6,7,8,9,10,11 },   // chromatic
            { 0,2,4,5,7,9,11 },              // major
            { 0,2,3,5,7,8,10 },              // natural minor
            { 0,2,4,7,9 },                   // major pentatonic
            { 0,3,5,7,10 }                   // minor pentatonic
        };
        const auto& d = degs[(size_t) juce::jlimit (0, 4, type)];
        const int base = 24 + juce::jlimit (0, 11, root);   // C1 + root
        std::vector<int> notes;
        for (int oct = 0; oct < 2; ++oct)
            for (int s : d) notes.push_back (base + 12 * oct + s);
        notes.push_back (base + 24);                          // top root
        return notes;
    }

    int gateIndex (float g)
    {
        int best = 0; float bd = 1.0e9f;
        for (int i = 0; i < 4; ++i) { const float e = std::abs (g - kGateVals[i]); if (e < bd) { bd = e; best = i; } }
        return best;
    }
}

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

    // Alternate the off-cell shade every 'groupSize' steps so the meter (4/4 or
    // 3/4) is visible on the pads themselves.
    auto groupShaded = [&] (int s) { return ((s / groupSize) % 2) == 1; };

    auto cell = [&] (juce::Rectangle<float> r, bool on, bool accent, bool playing, int s)
    {
        const juce::Colour off = groupShaded (s) ? Colors::grayOff.brighter (0.22f) : Colors::grayOff;
        g.setColour (on ? (accent ? Colors::yellow : Colors::orange) : off);
        g.fillRoundedRectangle (r.reduced (1.0f), 2.0f);
        if (playing) { g.setColour (Colors::white); g.drawRoundedRectangle (r.reduced (1.0f), 2.0f, 1.5f); }
    };

    if (mode == Mode::grid)
    {
        auto labels = area.removeFromLeft (kLabelW);
        const int   rows = numVoices + 1;                 // voices + accent lane
        const float cw = area.getWidth() / (float) len;
        const float rh = area.getHeight() / (float) rows;

        const float padW = (kLabelW - kNameW) / (float) kFillPads;
        for (int v = 0; v < numVoices; ++v)
        {
            const int rowY = (int) (area.getY() + v * rh);

            g.setColour (Colors::text);
            g.setFont (juce::FontOptions (9.5f));
            g.drawText (juce::String (voiceSpecs()[(size_t) v].prefix).toUpperCase(),
                        labels.getX(), rowY, kNameW - 2, (int) rh, juce::Justification::centredRight);

            // auto-fill pads 1..4 (fill every n steps)
            const float ph = juce::jmin ((float) rh - 3.0f, 13.0f);
            const float py = rowY + ((float) rh - ph) * 0.5f;
            for (int n = 1; n <= kFillPads; ++n)
            {
                juce::Rectangle<float> pr (labels.getX() + kNameW + (n - 1) * padW, py, padW - 2.0f, ph);
                g.setColour (Colors::panelLight.brighter (0.05f));
                g.fillRoundedRectangle (pr, 2.0f);
                g.setColour (Colors::cream.withAlpha (0.85f));
                g.setFont (juce::FontOptions (8.0f, juce::Font::bold));
                g.drawText (juce::String (n), pr, juce::Justification::centred);
            }

            for (int s = 0; s < len; ++s)
                cell ({ area.getX() + s * cw, area.getY() + v * rh, cw, rh },
                      seq.getStep (pat, editVar, v, s), seq.getAccent (pat, editVar, s), s == playStep, s);
        }

        // accent lane
        g.setColour (Colors::yellow);
        g.setFont (juce::FontOptions (10.0f));
        g.drawText ("ACC", labels.getX(), (int) (area.getY() + numVoices * rh), kLabelW - 4, (int) rh,
                    juce::Justification::centredRight);
        for (int s = 0; s < len; ++s)
            cell ({ area.getX() + s * cw, area.getY() + numVoices * rh, cw, rh },
                  seq.getAccent (pat, editVar, s), true, s == playStep, s);
    }
    else // authentic 808-style: instrument selector + edit-layer + step keys
    {
        juce::ignoreUnused (cell);
        const int  numInst = numVoices + 2;                // 16 voices + ACCENT + BASS
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
            const juce::String nm = (i == accentIndex) ? "ACC"
                                  : (i == bassIndex)   ? "BASS"
                                  : juce::String (voiceSpecs()[(size_t) i].prefix).toUpperCase();
            g.drawText (nm, r, juce::Justification::centred);
        }

        area.removeFromTop (4);

        if (selectedVoice == bassIndex) { paintBass (g, area); return; }   // tonal piano-roll

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
            const bool shade  = ((s / groupSize) % 2) == 1;
            const bool isDown = (s % groupSize == 0);
            const juce::Colour offCol = shade ? Colors::grayOff.brighter (0.22f) : Colors::grayOff;
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
        if (labels.contains (e.getPosition()))            // label area: name -> preview, pads -> auto-fill
        {
            const int row = (int) ((e.y - area.getY()) / rhAll);
            if (row >= 0 && row < numVoices)
            {
                if (e.x < labels.getX() + kNameW)
                {
                    if (onPreview) onPreview (row);
                }
                else
                {
                    const float padW = (kLabelW - kNameW) / (float) kFillPads;
                    const int n = (int) ((e.x - (labels.getX() + kNameW)) / padW) + 1;
                    if (n >= 1 && n <= kFillPads) applyFill (row, n);
                }
            }
            repaint();
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
        const int  numInst = numVoices + 2;
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

        if (selectedVoice == bassIndex) { mouseBass (e, area); return; }   // tonal piano-roll

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

//== Melodic BD BASS: tonal piano-roll =========================================
void StepSequencerView::paintBass (juce::Graphics& g, juce::Rectangle<int> area)
{
    const int pat = seq.getCurrentPattern();
    const int len = juce::jlimit (1, Sequencer::maxSteps, seq.getLength (pat, editVar));
    const int playStep = seq.getDisplayStep();

    // control strip: SCALE | ROOT | LEN (click to cycle)
    auto ctl = area.removeFromTop (juce::jmax (18, area.getHeight() / 8));
    const juce::String ctlText[3] = {
        juce::String ("SCALE ") + kScaleNames[juce::jlimit (0, 4, seq.getBassScale())],
        juce::String ("ROOT ")  + juce::MidiMessage::getMidiNoteName (24 + seq.getBassRoot(), true, false, 3),
        juce::String ("LEN ")   + kGateNames[gateIndex (seq.getBassGate())]
    };
    const float ctlW = ctl.getWidth() / 3.0f;
    for (int i = 0; i < 3; ++i)
    {
        juce::Rectangle<float> r (ctl.getX() + i * ctlW, (float) ctl.getY(), ctlW - 3.0f, (float) ctl.getHeight());
        g.setColour (Colors::panelLight);
        g.fillRoundedRectangle (r.reduced (1.0f), 3.0f);
        g.setColour (Colors::cream);
        g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.drawText (ctlText[i], r, juce::Justification::centred);
    }

    area.removeFromTop (4);

    // piano roll: rows = scale notes (2 octaves) at a fixed height, scrollable.
    const auto notes = bassScaleNotes (seq.getBassRoot(), seq.getBassScale());
    const int rows = (int) notes.size();
    bassRowsTotal = rows;
    if (rows <= 0) return;

    const int sbW = 9;
    auto sb     = area.removeFromRight (sbW);          // scrollbar gutter
    auto labels = area.removeFromLeft (34);
    const float cw   = area.getWidth() / (float) len;
    const int   rowH = kBassRowH;
    const int   visN = juce::jmax (1, area.getHeight() / rowH);
    const int   maxScroll = juce::jmax (0, rows - visN);
    bassScroll = juce::jlimit (0, maxScroll, bassScroll);

    for (int i = 0; i < visN && (bassScroll + i) < rows; ++i)
    {
        const int   dispIdx = bassScroll + i;
        const int   note = notes[(size_t) (rows - 1 - dispIdx)];   // high notes at the top
        const float y  = (float) (area.getY() + i * rowH);
        const bool  isRoot = ((note - seq.getBassRoot()) % 12 == 0);

        g.setColour (isRoot ? Colors::cream : Colors::text);
        g.setFont (juce::FontOptions (9.0f));
        g.drawText (juce::MidiMessage::getMidiNoteName (note, true, true, 3),
                    labels.getX(), (int) y, 34 - 3, rowH, juce::Justification::centredRight);

        for (int s = 0; s < len; ++s)
        {
            juce::Rectangle<float> cr (area.getX() + s * cw, y, cw, (float) rowH);
            const bool on = (seq.getBassNote (pat, editVar, s) == note);
            const bool shade = ((s / groupSize) % 2) == 1;
            g.setColour (on ? Colors::orange
                            : (isRoot ? Colors::grayOff.brighter (shade ? 0.30f : 0.12f)
                                      : (shade ? Colors::grayOff.brighter (0.18f) : Colors::grayOff)));
            g.fillRoundedRectangle (cr.reduced (0.8f), 2.0f);
            if (s == playStep) { g.setColour (Colors::white.withAlpha (0.5f)); g.drawRoundedRectangle (cr.reduced (0.8f), 2.0f, 1.0f); }
        }
    }

    // scrollbar
    if (maxScroll > 0)
    {
        g.setColour (Colors::background);
        g.fillRoundedRectangle (sb.toFloat().reduced (1.5f, 0.0f), 3.0f);
        const float thumbH = juce::jmax (18.0f, sb.getHeight() * (float) visN / (float) rows);
        const float ty = sb.getY() + (sb.getHeight() - thumbH) * (float) bassScroll / (float) maxScroll;
        g.setColour (Colors::cream.withAlpha (0.65f));
        g.fillRoundedRectangle (sb.getX() + 1.5f, ty, sb.getWidth() - 3.0f, thumbH, 3.0f);
    }
}

void StepSequencerView::mouseBass (const juce::MouseEvent& e, juce::Rectangle<int> area)
{
    const int pat = seq.getCurrentPattern();
    const int len = juce::jlimit (1, Sequencer::maxSteps, seq.getLength (pat, editVar));

    auto ctl = area.removeFromTop (juce::jmax (18, area.getHeight() / 8));
    if (ctl.contains (e.getPosition()))
    {
        const int third = juce::jlimit (0, 2, (int) ((e.x - ctl.getX()) / (ctl.getWidth() / 3.0f)));
        if (third == 0)      seq.setBassScale (seq.getBassRoot(), (seq.getBassScale() + 1) % 5);
        else if (third == 1) seq.setBassScale ((seq.getBassRoot() + 1) % 12, seq.getBassScale());
        else                 seq.setBassGate (kGateVals[(gateIndex (seq.getBassGate()) + 1) % 4]);
        repaint();
        return;
    }

    area.removeFromTop (4);

    const auto notes = bassScaleNotes (seq.getBassRoot(), seq.getBassScale());
    const int rows = (int) notes.size();
    if (rows <= 0) return;

    const int sbW = 9;
    auto sb     = area.removeFromRight (sbW);
    area.removeFromLeft (34);
    const float cw   = area.getWidth() / (float) len;
    const int   rowH = kBassRowH;
    const int   visN = juce::jmax (1, area.getHeight() / rowH);
    const int   maxScroll = juce::jmax (0, rows - visN);
    bassScroll = juce::jlimit (0, maxScroll, bassScroll);

    if (sb.contains (e.getPosition()))                      // click the scrollbar track to jump
    {
        if (maxScroll > 0)
            bassScroll = juce::jlimit (0, maxScroll,
                (int) std::lround ((double) (e.y - sb.getY()) / (double) sb.getHeight() * maxScroll));
        repaint();
        return;
    }

    const int s = (int) ((e.x - area.getX()) / cw);
    const int i = (int) ((e.y - area.getY()) / rowH);
    if (s < 0 || s >= len || i < 0 || i >= visN) return;
    const int dispIdx = bassScroll + i;
    if (dispIdx >= rows) return;

    const int note = notes[(size_t) (rows - 1 - dispIdx)];
    if (seq.getBassNote (pat, editVar, s) == note)
        seq.setBassNote (pat, editVar, s, -1);              // toggle off
    else
    {
        seq.setBassNote (pat, editVar, s, note);
        if (onPreviewBass) onPreviewBass (note);            // audition the note
    }
    repaint();
}

void StepSequencerView::applyFill (int voice, int n)
{
    if (voice < 0 || voice >= numVoices || n < 1) return;
    const int pat = seq.getCurrentPattern();
    const int len = juce::jlimit (1, Sequencer::maxSteps, seq.getLength (pat, editVar));

    // Toggle: if the row already equals the every-n pattern, clear it instead.
    bool matches = true;
    for (int s = 0; s < len; ++s)
        if (seq.getStep (pat, editVar, voice, s) != (s % n == 0)) { matches = false; break; }
    for (int s = 0; s < len; ++s)
        seq.setStep (pat, editVar, voice, s, matches ? false : (s % n == 0));
}

void StepSequencerView::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    if (selectedVoice != bassIndex) return;
    const int delta = (int) std::lround (w.deltaY * 3.0);
    if (delta == 0) return;
    bassScroll = juce::jlimit (0, juce::jmax (0, bassRowsTotal - 1), bassScroll - delta);
    repaint();
}
}
