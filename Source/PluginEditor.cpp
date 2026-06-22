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

    setupPresetBox (kitBox, PresetManager::kitNames());
    kitBox.onChange = [this] { handleKitBox(); };
    setupPresetBox (patternBox, PresetManager::patternNames());
    patternBox.onChange = [this] { handlePatternBox(); };
    kitLabel.setFont (juce::FontOptions (10.0f));
    patternLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (kitBox); addAndMakeVisible (patternBox);
    addAndMakeVisible (kitLabel); addAndMakeVisible (patternLabel);

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
    accentKnob = std::make_unique<ParamKnob> (proc.apvts, ParamIDs::accentLevel, "ACCENT");
    addAndMakeVisible (*masterGainKnob); addAndMakeVisible (*masterDriveKnob);
    addAndMakeVisible (*accentKnob); addAndMakeVisible (*multiOutToggle);

    lenBox.addItem ("16", 1); lenBox.addItem ("32", 2); lenBox.addItem ("8", 3);
    lenBox.onChange = [this]
    {
        const int id = lenBox.getSelectedId();
        const int L = (id == 2 ? 32 : id == 3 ? 8 : 16);
        const int pat = proc.getSequencer().getCurrentPattern();
        proc.getSequencer().setLength (pat, 0, L);
        proc.getSequencer().setLength (pat, 1, L);
        stepView.repaint();
    };
    lenLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (lenBox); addAndMakeVisible (lenLabel);

    tripButton.setClickingTogglesState (true);
    tripButton.onClick = [this]
    {
        const bool trip = tripButton.getToggleState();
        const int pat = proc.getSequencer().getCurrentPattern();
        proc.getSequencer().setStepDiv (pat, 0, trip ? (1.0f / 6.0f) : 0.25f);
        proc.getSequencer().setStepDiv (pat, 1, trip ? (1.0f / 6.0f) : 0.25f);
        tripButton.setButtonText (trip ? "1/16T" : "1/16");
    };
    addAndMakeVisible (tripButton);

    for (int i = 1; i <= Sequencer::numPatterns; ++i) patBox.addItem (juce::String (i), i);
    patBox.onChange = [this]
    {
        proc.getSequencer().setCurrentPattern (patBox.getSelectedId() - 1);
        syncTransport();
        stepView.repaint();
    };
    patLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (patBox); addAndMakeVisible (patLabel);

    songButton.setClickingTogglesState (true);
    songButton.setColour (juce::TextButton::buttonOnColourId, Colors::orange);
    songButton.onClick = [this] { proc.getSequencer().setChainEnabled (songButton.getToggleState()); };
    addAndMakeVisible (songButton);

    chainEditor.setTextToShowWhenEmpty ("e.g. 1 2 1 3", Colors::grayOff);
    chainEditor.setTooltip ("Song / chain: pattern numbers (1-8) played in order when SONG is on");
    auto applyChain = [this]
    {
        const auto toks = juce::StringArray::fromTokens (chainEditor.getText(), " ,", "");
        std::vector<int> c;
        for (auto& t : toks)
            if (t.trim().isNotEmpty())
            {
                const int v = t.getIntValue();
                if (v >= 1 && v <= Sequencer::numPatterns) c.push_back (v - 1);
            }
        proc.getSequencer().setChain (c);
    };
    chainEditor.onReturnKey = applyChain;
    chainEditor.onFocusLost = applyChain;
    addAndMakeVisible (chainEditor);

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
    stepView.onSelect = [this] (int v)
    {
        selectedVoice = v;
        if (v >= 0 && v < numVoices) buildEditFor (v);   // accent (== numVoices) has no edit panel
    };

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

void TR808AudioProcessorEditor::setupPresetBox (juce::ComboBox& box, const juce::StringArray& factory)
{
    box.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& n : factory) box.addItem (n, id++);
    box.addSeparator();
    box.addItem ("Save\xE2\x80\xA6", 100);
    box.addItem ("Load\xE2\x80\xA6", 101);
    box.setTextWhenNothingSelected ("\xE2\x80\x94");
}

void TR808AudioProcessorEditor::handleKitBox()
{
    const int id = kitBox.getSelectedId();
    if (id == 0) return;

    if (id == 100)
    {
        chooser = std::make_unique<juce::FileChooser> ("Save Kit", PresetManager::presetsDir(), "*.xml");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f != juce::File()) PresetManager::saveKit (proc.apvts, f.withFileExtension ("xml")); });
    }
    else if (id == 101)
    {
        chooser = std::make_unique<juce::FileChooser> ("Load Kit", PresetManager::presetsDir(), "*.xml");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f.existsAsFile()) PresetManager::loadKit (proc.apvts, f); });
    }
    else
    {
        PresetManager::applyFactoryKit (proc.apvts, id - 1);
    }
    kitBox.setSelectedId (0, juce::dontSendNotification);
}

void TR808AudioProcessorEditor::handlePatternBox()
{
    const int id = patternBox.getSelectedId();
    if (id == 0) return;

    if (id == 100)
    {
        chooser = std::make_unique<juce::FileChooser> ("Save Pattern", PresetManager::presetsDir(), "*.xml");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f != juce::File()) PresetManager::savePattern (proc.getSequencer(), f.withFileExtension ("xml")); });
    }
    else if (id == 101)
    {
        chooser = std::make_unique<juce::FileChooser> ("Load Pattern", PresetManager::presetsDir(), "*.xml");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f.existsAsFile()) PresetManager::loadPattern (proc.getSequencer(), f); });
    }
    else
    {
        PresetManager::applyFactoryPattern (proc.getSequencer(), id - 1);
    }
    patternBox.setSelectedId (0, juce::dontSendNotification);
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

    const int L = seq.getLength (seq.getCurrentPattern(), 0);
    lenBox.setSelectedId (L == 32 ? 2 : L == 8 ? 3 : 1, juce::dontSendNotification);
    const bool trip = seq.getStepDiv (seq.getCurrentPattern(), 0) < 0.22f;
    tripButton.setToggleState (trip, juce::dontSendNotification);
    tripButton.setButtonText (trip ? "1/16T" : "1/16");

    patBox.setSelectedId (seq.getCurrentPattern() + 1, juce::dontSendNotification);

    songButton.setToggleState (seq.isChainEnabled(), juce::dontSendNotification);
    juce::String chainStr;
    for (int c : seq.getChain()) chainStr += juce::String (c + 1) + " ";
    chainEditor.setText (chainStr.trim(), juce::dontSendNotification);
}

void TR808AudioProcessorEditor::paint (juce::Graphics& g)
{
    // Charcoal metallic 808 chassis.
    auto bf = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2e2e2e), 0.0f, 0.0f,
                                             juce::Colour (0xff161616), 0.0f, bf.getHeight(), false));
    g.fillRect (bf);

    // header band + the signature TR-808 orange/cream stripe beneath it
    g.setColour (juce::Colour (0xff202020));
    g.fillRect (0, 0, getWidth(), 112);
    g.setColour (Colors::orange); g.fillRect (0, 108, getWidth(), 3);
    g.setColour (Colors::cream);  g.fillRect (0, 111, getWidth(), 1);

    // frame the rhythm (step) section like the 808 panel
    if (! stepBounds.isEmpty())
    {
        const auto sf = stepBounds.toFloat().expanded (4.0f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (sf, 5.0f);
        g.setColour (Colors::orange.withAlpha (0.5f));
        g.drawRoundedRectangle (sf, 5.0f, 1.0f);
    }
}

void TR808AudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // --- header (two rows) ---
    auto headerFull = area.removeFromTop (112);
    auto row1 = headerFull.removeFromTop (58).reduced (6, 4);
    auto row2 = headerFull.reduced (6, 4);

    // row 1: transport + master
    titleLabel.setBounds (row1.removeFromLeft (78));
    playButton.setBounds (row1.removeFromLeft (56).reduced (2, 8));
    auto tcol = row1.removeFromLeft (150);
    tempoLabel.setBounds (tcol.removeFromTop (13));
    tempoSlider.setBounds (tcol);
    auto scol = row1.removeFromLeft (150);
    swingLabel.setBounds (scol.removeFromTop (13));
    swingSlider.setBounds (scol);
    abModeBox.setBounds (row1.removeFromLeft (56).reduced (2, 12));
    patLabel.setBounds (row1.removeFromLeft (24));
    patBox.setBounds (row1.removeFromLeft (44).reduced (2, 12));
    if (masterGainKnob)  masterGainKnob->setBounds (row1.removeFromRight (54));
    if (masterDriveKnob) masterDriveKnob->setBounds (row1.removeFromRight (54));
    if (accentKnob)      accentKnob->setBounds (row1.removeFromRight (54));
    if (multiOutToggle)  multiOutToggle->setBounds (row1.removeFromRight (52).reduced (2, 12));

    // row 2: presets + length/triplet + view/grid/variation
    kitLabel.setBounds (row2.removeFromLeft (26));
    kitBox.setBounds (row2.removeFromLeft (116).reduced (2, 8));
    patternLabel.setBounds (row2.removeFromLeft (50));
    patternBox.setBounds (row2.removeFromLeft (116).reduced (2, 8));
    lenLabel.setBounds (row2.removeFromLeft (24));
    lenBox.setBounds (row2.removeFromLeft (46).reduced (2, 8));
    tripButton.setBounds (row2.removeFromLeft (52).reduced (2, 8));
    songButton.setBounds (row2.removeFromLeft (50).reduced (2, 8));
    chainEditor.setBounds (row2.removeFromLeft (92).reduced (2, 10));
    viewButton.setBounds (row2.removeFromRight (58).reduced (2, 8));
    gridButton.setBounds (row2.removeFromRight (58).reduced (2, 8));
    varBButton.setBounds (row2.removeFromRight (26).reduced (2, 8));
    varAButton.setBounds (row2.removeFromRight (26).reduced (2, 8));
    varLabel.setBounds (row2.removeFromRight (34).reduced (0, 8));

    // --- step grid at the bottom ---
    auto stepArea = area.removeFromBottom (juce::jmax (170, area.getHeight() * 40 / 100));
    stepBounds = stepArea.reduced (6);
    stepView.setBounds (stepBounds);

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
