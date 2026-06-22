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
    enum class Mode { grid, authentic };

    explicit StepSequencerView (Sequencer& sequencer);
    ~StepSequencerView() override;

    // selectedVoice in [0, numVoices): a voice; == numVoices: the ACCENT track.
    static constexpr int accentIndex = numVoices;

    void setMode (Mode m)            { mode = m; repaint(); }
    void setSelectedVoice (int v)    { selectedVoice = v; repaint(); }
    int  getSelectedVoice() const    { return selectedVoice; }
    void setEditVariation (int v)    { editVar = v; repaint(); }

    // Called when the authentic-view instrument selector picks an instrument
    // (0..numVoices, where numVoices == ACCENT).
    std::function<void (int)> onSelect;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    juce::Rectangle<int> gridArea() const;

    Sequencer& seq;
    Mode mode = Mode::grid;
    int  selectedVoice = BD;
    int  editVar = 0;
    int  lastDisplay = -2;

    static constexpr int kLabelW = 58;
};
}
