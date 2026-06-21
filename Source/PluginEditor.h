#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <functional>

#include "PluginProcessor.h"
#include "params/ParameterIDs.h"
#include "engine/VoiceDefs.h"
#include "engine/DeepParams.h"
#include "engine/PresetManager.h"
#include "ui/LookAndFeel808.h"
#include "ui/ParamComponents.h"
#include "ui/StepSequencerView.h"

//==============================================================================
// Full M6 editor: a top transport/master bar, a PERFORM panel (per-voice macro
// knobs + pan + mute/solo), an EDIT view (all deep params for the selected
// voice), and the step sequencer (Grid / Authentic). Everything APVTS-bound via
// attachments (no desync); transport talks to the Sequencer directly.
//==============================================================================
class TR808AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit TR808AudioProcessorEditor (TR808AudioProcessor&);
    ~TR808AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // One PERFORM column for a voice: a select button, macro + pan knobs, M/S.
    struct VoiceColumn : juce::Component
    {
        VoiceColumn (juce::AudioProcessorValueTreeState&, int voiceIndex, std::function<void (int)> onSelect);
        void resized() override;
        void paint (juce::Graphics&) override;

        int voiceIndex;
        juce::TextButton selectButton;
        juce::OwnedArray<tr808::ui::ParamKnob> knobs;
        std::unique_ptr<tr808::ui::ParamToggle> mute, solo;
    };

    void buildPerform();
    void buildEditFor (int voice);
    void showEdit (bool edit);
    void syncTransport();
    void setupPresetBox (juce::ComboBox&, const juce::StringArray& factory);
    void handleKitBox();
    void handlePatternBox();

    TR808AudioProcessor&       proc;
    tr808::ui::LookAndFeel808  lnf;

    juce::Label      titleLabel;
    juce::TextButton playButton  { "PLAY" };
    juce::TextButton viewButton  { "EDIT" };
    juce::TextButton gridButton  { "GRID" };
    juce::TextButton varAButton  { "A" }, varBButton { "B" };
    juce::ComboBox   abModeBox;
    juce::ComboBox   kitBox, patternBox, lenBox;
    juce::TextButton tripButton { "1/16" };
    juce::Label      kitLabel { {}, "KIT" }, patternLabel { {}, "PATTERN" }, lenLabel { {}, "LEN" };
    std::unique_ptr<juce::FileChooser> chooser;
    juce::Slider     tempoSlider, swingSlider;
    juce::Label      tempoLabel { {}, "TEMPO" }, swingLabel { {}, "SWING" }, varLabel { {}, "EDIT" };

    std::unique_ptr<tr808::ui::ParamKnob>   masterGainKnob, masterDriveKnob, accentKnob;
    std::unique_ptr<tr808::ui::ParamToggle> multiOutToggle;

    juce::Viewport  performViewport;
    juce::Component performPanel;
    juce::OwnedArray<VoiceColumn> voiceColumns;

    juce::Viewport  editViewport;
    juce::Component editPanel;
    juce::OwnedArray<juce::Component> editControls;
    juce::Label     editTitle;

    tr808::ui::StepSequencerView stepView;

    bool editMode = false;
    int  selectedVoice = tr808::BD;
    int  editVariation = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TR808AudioProcessorEditor)
};
