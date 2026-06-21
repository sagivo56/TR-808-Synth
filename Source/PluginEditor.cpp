#include "PluginEditor.h"

using namespace tr808;
using namespace tr808::ui;

//==============================================================================
TR808AudioProcessorEditor::VoiceColumn::VoiceColumn (juce::AudioProcessorValueTreeState& apvts,
                                                     int v, std::function<void (int)> onSelect)
    : voiceIndex (v)
{
    const auto& s = voiceSpecs()[(size_t) v];

    selectButton.setButtonText (juce::String (s.prefix).toUpperCase());
    selectButton.onClick = [this, onSelect] { if (onSelect) onSelect (voiceIndex); };
    addAndMakeVisible (selectButton);

    knobs.add (new ParamKnob (apvts, macroId (v, "level"), "LVL"));
    if (s.tone)   knobs.add (new ParamKnob (apvts, macroId (v, "tone"),   "TONE"));
    if (s.decay)  knobs.add (new ParamKnob (apvts, macroId (v, "decay"),  "DEC"));
    if (s.snappy) knobs.add (new ParamKnob (apvts, macroId (v, "snappy"), "SNP"));
    if (s.tune)   knobs.add (new ParamKnob (apvts, macroId (v, "tune"),   "TUNE"));
    knobs.add (new ParamKnob (apvts, macroId (v, "pan"), "PAN"));
    for (auto* k : knobs) addAndMakeVisible (k);

    mute = std::make_unique<ParamToggle> (apvts, macroId (v, "mute"), "M", Colors::red);
    solo = std::make_unique<ParamToggle> (apvts, macroId (v, "solo"), "S", Colors::yellow);
    addAndMakeVisible (*mute);
    addAndMakeVisible (*solo);
}

void TR808AudioProcessorEditor::VoiceColumn::paint (juce::Graphics& g)
{
    g.setColour (Colors::panel);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 3.0f);
}

void TR808AudioProcessorEditor::VoiceColumn::resized()
{
    auto r = getLocalBounds().reduced (2);
    selectButton.setBounds (r.removeFromTop (18));

    auto ms = r.removeFromBottom (18);
    mute->setBounds (ms.removeFromLeft (ms.getWidth() / 2).reduced (1));
    solo->setBounds (ms.reduced (1));

    const int n = knobs.size();
    const int kh = n > 0 ? r.getHeight() / n : r.getHeight();
    for (auto* k : knobs)
        k->setBounds (r.removeFromTop (kh));
}

//==============================================================================
TR808AudioProcessorEditor::TR808AudioProcessorEditor (TR808AudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), stepView (p.getSequencer())
{
    setLookAndFeel (&lnf);

    titleLabel.setText ("TR-808", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, Colors::orange);
    addAndMakeVisible (titleLabel);

    playButton.setClickingTogglesState (true);
    playButton.setColour (juce::TextButton::buttonOnColourId, Colors::orange);
    playButton.onClick = [this] { proc.getSequencer().setRunning (playButton.getToggleState()); };
    addAndMakeVisible (playButton);

    viewButton.onClick = [this] { showEdit (! editMode); };
    addAndMakeVisible (viewButton);

    gridButton.setClickingTogglesState (true);
    gridButton.onClick = [this]
    {
        const bool auth = gridButton.getToggleState();
        stepView.setMode (auth ? StepSequencerView::Mode::authentic : StepSequencerView::Mode::grid);
        gridButton.setButtonText (auth ? "AUTH" : "GRID");
    };
    addAndMakeVisible (gridButton);

    varAButton.setClickingTogglesState (true);
    varBButton.setClickingTogglesState (true);
    varAButton.setToggleState (true, juce::dontSendNotification);
    varAButton.onClick = [this] { editVariation = 0; varAButton.setToggleState (true, juce::dontSendNotification);
                                  varBButton.setToggleState (false, juce::dontSendNotification); stepView.setEditVariation (0); };
    varBButton.onClick = [this] { editVariation = 1; varBButton.setToggleState (true, juce::dontSendNotification);
                                  varAButton.setToggleState (false, juce::dontSendNotification); stepView.setEditVariation (1); };
    varLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (varLabel); addAndMakeVisible (varAButton); addAndMakeVisible (varBButton);

    abModeBox.addItem ("A", 1); abModeBox.addItem ("B", 2); abModeBox.addItem ("AB", 3);
    abModeBox.onChange = [this]
    {
        const int m = abModeBox.getSelectedId();
        proc.getSequencer().setPlayMode (m == 1 ? Sequencer::PlayMode::a
                                       : m == 2 ? Sequencer::PlayMode::b
                                                : Sequencer::PlayMode::ab);
    };
    addAndMakeVisible (abModeBox);

    tempoSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    tempoSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 44, 16);
    tempoSlider.setRange (20.0, 300.0, 1.0);
    tempoSlider.onValueChange = [this] { proc.getSequencer().setTempo (tempoSlider.getValue()); };
    tempoLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (tempoSlider); addAndMakeVisible (tempoLabel);

    swingSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    swingSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 44, 16);
    swingSlider.setRange (0.0, 0.75, 0.01);
    swingSlider.onValueChange = [this] { proc.getSequencer().setSwing ((float) swingSlider.getValue()); };
    swingLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (swingSlider); addAndMakeVisible (swingLabel);

    masterGainKnob  = std::make_unique<ParamKnob>   (proc.apvts, ParamIDs::masterGain,  "MAIN");
    masterDriveKnob = std::make_unique<ParamKnob>   (proc.apvts, ParamIDs::masterDrive, "DRIVE");
    multiOutToggle  = std::make_unique<ParamToggle> (proc.apvts, ParamIDs::multiOut,    "MULTI", Colors::orange);
    addAndMakeVisible (*masterGainKnob); addAndMakeVisible (*masterDriveKnob); addAndMakeVisible (*multiOutToggle);

    performViewport.setViewedComponent (&performPanel, false);
    performViewport.setScrollBarsShown (false, true);
    addAndMakeVisible (performViewport);
    buildPerform();

    editViewport.setViewedComponent (&editPanel, false);
    editViewport.setScrollBarsShown (true, false);
    addChildComponent (editViewport);
    editTitle.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    editTitle.setColour (juce::Label::textColourId, Colors::orange);
    addChildComponent (editTitle);
    buildEditFor (selectedVoice);

    addAndMakeVisible (stepView);
    stepView.setSelectedVoice (selectedVoice);

    syncTransport();
    showEdit (false);

    setResizable (true, true);
    setResizeLimits (900, 560, 2400, 1500);
    setSize (1180, 720);
}

TR808AudioProcessorEditor::~TR808AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void TR808AudioProcessorEditor::buildPerform()
{
    voiceColumns.clear();
    for (int v = 0; v < numVoices; ++v)
    {
        auto* col = new VoiceColumn (proc.apvts, v, [this] (int vi)
        {
            selectedVoice = vi;
            stepView.setSelectedVoice (vi);
            buildEditFor (vi);
            showEdit (true);
        });
        voiceColumns.add (col);
        performPanel.addAndMakeVisible (col);
    }
}

void TR808AudioProcessorEditor::buildEditFor (int voice)
{
    editControls.clear();
    editTitle.setText (juce::String (voiceSpecs()[(size_t) voice].name) + "  \xE2\x80\x94  DEEP EDIT", juce::dontSendNotification);

    auto addKnob = [&] (const std::string& id, const juce::String& label)
    {
        auto* k = new ParamKnob (proc.apvts, id, label);
        editControls.add (k);
        editPanel.addAndMakeVisible (k);
    };

    const auto& s = voiceSpecs()[(size_t) voice];
    addKnob (macroId (voice, "level"), "Level");
    if (s.tone)   addKnob (macroId (voice, "tone"),   "Tone");
    if (s.decay)  addKnob (macroId (voice, "decay"),  "Decay");
    if (s.snappy) addKnob (macroId (voice, "snappy"), "Snappy");
    if (s.tune)   addKnob (macroId (voice, "tune"),   "Tune");
    addKnob (macroId (voice, "pan"), "Pan");

    for (const auto& d : deepParamDescs())
        if (d.voice == voice)
            addKnob (macroId (voice, d.suffix), d.label);

    auto* mt = new ParamToggle (proc.apvts, macroId (voice, "mute"), "MUTE", Colors::red);
    auto* st = new ParamToggle (proc.apvts, macroId (voice, "solo"), "SOLO", Colors::yellow);
    editControls.add (mt); editControls.add (st);
    editPanel.addAndMakeVisible (mt);
    editPanel.addAndMakeVisible (st);

    resized();
}

void TR808AudioProcessorEditor::showEdit (bool edit)
{
    editMode = edit;
    performViewport.setVisible (! edit);
    editViewport.setVisible (edit);
    editTitle.setVisible (edit);
    viewButton.setButtonText (edit ? "PERF" : "EDIT");
    resized();
}

void TR808AudioProcessorEditor::syncTransport()
{
    auto& seq = proc.getSequencer();
    playButton.setToggleState (seq.isRunning(), juce::dontSendNotification);
    tempoSlider.setValue (seq.getTempo(), juce::dontSendNotification);
    swingSlider.setValue (seq.getSwing(), juce::dontSendNotification);
    const auto pm = seq.getPlayMode();
    abModeBox.setSelectedId (pm == Sequencer::PlayMode::a ? 1 : pm == Sequencer::PlayMode::b ? 2 : 3,
                             juce::dontSendNotification);
}

void TR808AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colors::background);
    g.setColour (Colors::panelLight);
    g.fillRect (getLocalBounds().removeFromTop (76));
}

void TR808AudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // --- header ---
    auto header = area.removeFromTop (76).reduced (6);
    titleLabel.setBounds (header.removeFromLeft (84));
    playButton.setBounds (header.removeFromLeft (58).reduced (2, 16));

    auto tcol = header.removeFromLeft (150);
    tempoLabel.setBounds (tcol.removeFromTop (14));
    tempoSlider.setBounds (tcol.reduced (0, 2));
    auto scol = header.removeFromLeft (150);
    swingLabel.setBounds (scol.removeFromTop (14));
    swingSlider.setBounds (scol.reduced (0, 2));
    abModeBox.setBounds (header.removeFromLeft (64).reduced (2, 22));

    // right-aligned cluster
    if (masterGainKnob)  masterGainKnob->setBounds (header.removeFromRight (62));
    if (masterDriveKnob) masterDriveKnob->setBounds (header.removeFromRight (62));
    if (multiOutToggle)  multiOutToggle->setBounds (header.removeFromRight (58).reduced (2, 22));
    viewButton.setBounds (header.removeFromRight (58).reduced (2, 22));
    gridButton.setBounds (header.removeFromRight (58).reduced (2, 22));
    varBButton.setBounds (header.removeFromRight (26).reduced (2, 22));
    varAButton.setBounds (header.removeFromRight (26).reduced (2, 22));
    varLabel.setBounds (header.removeFromRight (34).reduced (0, 22));

    // --- step grid at the bottom ---
    auto stepArea = area.removeFromBottom (juce::jmax (170, area.getHeight() * 40 / 100));
    stepView.setBounds (stepArea.reduced (6));

    // --- middle: PERFORM or EDIT ---
    auto mid = area.reduced (6);

    performViewport.setBounds (mid);
    {
        const int colW = 90;
        const int ph = juce::jmax (120, performViewport.getMaximumVisibleHeight());
        performPanel.setSize (numVoices * colW, ph);
        for (int v = 0; v < voiceColumns.size(); ++v)
            voiceColumns[v]->setBounds (v * colW, 0, colW, ph);
    }

    auto editArea = mid;
    editTitle.setBounds (editArea.removeFromTop (20));
    editViewport.setBounds (editArea);
    {
        const int cw = 94, ch = 98;
        const int cols = juce::jmax (1, editViewport.getMaximumVisibleWidth() / cw);
        const int n = editControls.size();
        const int rows = (n + cols - 1) / juce::jmax (1, cols);
        editPanel.setSize (juce::jmax (cw, editViewport.getMaximumVisibleWidth()), juce::jmax (ch, rows * ch));
        for (int i = 0; i < n; ++i)
            editControls[i]->setBounds ((i % cols) * cw, (i / cols) * ch, cw - 4, ch - 4);
    }
}
