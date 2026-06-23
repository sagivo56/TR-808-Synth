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
    bool keyPressed (const juce::KeyPress&) override;   // SPACE toggles transport
    void mouseDown (const juce::MouseEvent&) override { grabKeyboardFocus(); }

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

    // FX panel background: three tinted bands - REVERB (cool), DELAY (warm),
    // DRIVE (green) - separated at split1 and split2.
    struct ShadedPanel : juce::Component
    {
        int split1 = 114, split2 = 228;
        void paint (juce::Graphics&) override;
    };

    void buildPerform();
    void buildEditFor (int voice);
    void buildBassEdit();
    void buildFx();
    void selectEditVar (int v);   // choose which variation (0..3 = A..D) the grid edits
    void showEdit (bool edit);
    void showFx (bool fx);
    void syncTransport();
    void setupPresetBox (juce::ComboBox&, const juce::StringArray& factory, const juce::String& placeholder);
    void handleKitBox();
    void handlePatternBox();

    TR808AudioProcessor&       proc;
    tr808::ui::LookAndFeel808  lnf;

    juce::Label      titleLabel;
    juce::TextButton playButton  { "PLAY" };
    juce::TextButton viewButton  { "EDIT" };
    juce::TextButton gridButton  { "GRID" };
    juce::TextButton bassButton  { "BD BASS" };
    juce::TextButton varAButton  { "A" }, varBButton { "B" }, varCButton { "C" }, varDButton { "D" };
    juce::ComboBox   abModeBox;
    juce::ComboBox   copyBox;     // copy the edited variation onto A/B/C/D
    juce::ComboBox   kitBox, patternBox, lenBox, patBox, sigBox;
    juce::TextButton tripButton { "1/16" };
    juce::TextButton songButton { "SONG" };
    juce::TextButton clearButton { "CLR" };
    juce::TextEditor chainEditor;
    juce::Label      kitLabel { {}, "KIT" }, patternLabel { {}, "PATTERN" }, lenLabel { {}, "LEN" }, patLabel { {}, "PAT" }, sigLabel { {}, "SIG" };
    std::unique_ptr<juce::FileChooser> chooser;
    juce::File lastKitFile, lastPatternFile;   // for the single "Save" (update last Save As)
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

    juce::TextButton fxButton { "FX" };
    ShadedPanel      fxPanel;
    juce::OwnedArray<tr808::ui::ParamKnob> fxControls;
    juce::Label      fxTitle { {}, "FX  -  REVERB (Lexicon-style) + PING-PONG DELAY" };
    juce::Label      fxRevLabel { {}, "REVERB" }, fxDlyLabel { {}, "DELAY" }, fxDrvLabel { {}, "DRIVE" };
    int              fxDelayStart = 0;   // index in fxControls where delay knobs begin
    int              fxDriveStart = 0;   // index where drive knobs begin
    bool             fxMode = false;

    tr808::ui::StepSequencerView stepView;

    int  headerH = 112;        // header band height (taller in compact/mobile mode)
    bool editMode = false;
    int  selectedVoice = tr808::BD;
    int  editVariation = 0;
    juce::Rectangle<int> stepBounds;   // rhythm-section frame (set in resized, drawn in paint)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TR808AudioProcessorEditor)
};
