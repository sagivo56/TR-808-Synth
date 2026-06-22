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

    // selectedVoice in [0, numVoices): a voice; == numVoices: the ACCENT track.
    static constexpr int accentIndex = numVoices;

    void setMode (Mode m)            { mode = m; repaint(); }
    void setSelectedVoice (int v)    { selectedVoice = v; repaint(); }
    int  getSelectedVoice() const    { return selectedVoice; }
    void setEditVariation (int v)    { editVar = v; repaint(); }

    // Beat grouping for the visible bar lines (stepsPerBeat is always 4 = 16ths).
    void setBeatsPerBar (int n)      { beatsPerBar = juce::jlimit (1, 8, n); repaint(); }

    // Called when the authentic-view instrument selector picks an instrument
    // (0..numVoices, where numVoices == ACCENT).
    std::function<void (int)> onSelect;

    // Called to audition an instrument (0..numVoices-1) when its name is clicked.
    std::function<void (int)> onPreview;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    juce::Rectangle<int> gridArea() const;

    Sequencer& seq;
    Mode  mode = Mode::grid;
    Layer layer = Layer::step;
    int  selectedVoice = BD;
    int  editVar = 0;
    int  lastDisplay = -2;
    int  beatsPerBar = 4;                  // 4 = 4/4, 3 = 3/4 (visible grouping)
    static constexpr int stepsPerBeat = 4; // 16th-note steps

    static constexpr int kLabelW = 58;
};
}
