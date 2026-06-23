#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "../engine/Sequencer.h"
#include "../engine/VoiceDefs.h"
#include "LookAndFeel808.h"

namespace tr808::ui
{
// The step grid. Two modes: Grid (all 16 voices x 16 steps) and Authentic
// (one selected voice's 16 steps + the accent row). Edits the Sequencer
// directly and shows a moving playhead (polled via a timer).
class StepSequencerView : public juce::Component, private juce::Timer
{
public:
    enum class Mode  { grid, authentic };
    enum class Layer { step, flam, prob };   // what the step keys edit (authentic view)

    explicit StepSequencerView (Sequencer& sequencer);
    ~StepSequencerView() override;

    // selectedVoice in [0, numVoices): a voice; == numVoices: ACCENT track;
    // == numVoices+1: the melodic BD BASS (tonal piano-roll editor).
    static constexpr int accentIndex = numVoices;
    static constexpr int bassIndex   = numVoices + 1;

    void setMode (Mode m)            { mode = m; repaint(); }
    void setSelectedVoice (int v)    { selectedVoice = v; repaint(); }
    int  getSelectedVoice() const    { return selectedVoice; }
    void setEditVariation (int v)    { editVar = v; repaint(); }

    // Beat grouping: alternate cell shading every 'n' steps (4 = 4/4, 3 = 3/4).
    void setGrouping (int n)         { groupSize = juce::jlimit (1, 8, n); repaint(); }

    // Called when the authentic-view instrument selector picks an instrument
    // (0..numVoices, where numVoices == ACCENT).
    std::function<void (int)> onSelect;

    // Called to audition an instrument (0..numVoices-1) when its name is clicked.
    std::function<void (int)> onPreview;
    // Called to audition a bass MIDI note when a piano-roll cell is clicked.
    std::function<void (int)> onPreviewBass;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    void timerCallback() override;
    juce::Rectangle<int> gridArea() const;
    void paintBass (juce::Graphics&, juce::Rectangle<int> area);
    void mouseBass (const juce::MouseEvent&, juce::Rectangle<int> area);
    void applyFill (int voice, int n);    // auto-fill a row every n steps (toggle)

    Sequencer& seq;
    Mode  mode = Mode::grid;
    Layer layer = Layer::step;
    int  selectedVoice = BD;
    int  editVar = 0;
    int  lastDisplay = -2;
    int  groupSize = 4;                    // shade alternates every this many steps (4=4/4, 3=3/4)
    int  bassScroll = 100000;              // first visible piano-roll row (clamped per paint)
    int  bassRowsTotal = 0;                // total scale rows (set in paintBass)

    static constexpr int kLabelW    = 96;  // wider: instrument name + 4 fill pads
    static constexpr int kNameW     = 38;  // name sub-column inside the label area
    static constexpr int kBassRowH  = 18;  // fixed piano-roll row height (scrollable)
    static constexpr int kFillPads  = 4;   // auto-fill pads 1..4
};
}
