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
    knobs.add (new ParamKnob (apvts, macroId (v, "revsend"), "RVB"));
    knobs.add (new ParamKnob (apvts, macroId (v, "dlysend"), "DLY"));
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

    // Two knobs per row (side by side) so the extra send knobs fit without a
    // very tall column.
    const int n = knobs.size();
    const int rows = (n + 1) / 2;
    const int kh = rows > 0 ? r.getHeight() / rows : r.getHeight();
    const int colW = r.getWidth() / 2;
    for (int i = 0; i < n; ++i)
        knobs[i]->setBounds (r.getX() + (i % 2) * colW, r.getY() + (i / 2) * kh, colW, kh);
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

    fxButton.setClickingTogglesState (true);
    fxButton.setColour (juce::TextButton::buttonOnColourId, Colors::orange);
    fxButton.onClick = [this] { showFx (fxButton.getToggleState()); };
    addAndMakeVisible (fxButton);

    gridButton.setClickingTogglesState (true);
    gridButton.onClick = [this]
    {
        const bool auth = gridButton.getToggleState();
        stepView.setMode (auth ? StepSequencerView::Mode::authentic : StepSequencerView::Mode::grid);
        gridButton.setButtonText (auth ? "AUTH" : "GRID");
        if (! auth && bassButton.getToggleState())   // leaving auth cancels bass view
            bassButton.setToggleState (false, juce::sendNotification);
    };
    addAndMakeVisible (gridButton);

    // BD BASS: open the melodic tonal grid (authentic view, BASS instrument).
    bassButton.setClickingTogglesState (true);
    bassButton.setColour (juce::TextButton::buttonOnColourId, Colors::orange);
    bassButton.onClick = [this]
    {
        if (bassButton.getToggleState())
        {
            gridButton.setToggleState (true, juce::dontSendNotification);
            gridButton.setButtonText ("AUTH");
            stepView.setMode (StepSequencerView::Mode::authentic);
            stepView.setSelectedVoice (StepSequencerView::bassIndex);
            selectedVoice = StepSequencerView::bassIndex;
            buildBassEdit();
            showEdit (true);                         // open the bass controls too
        }
        else
        {
            selectedVoice = BD;
            stepView.setSelectedVoice (BD);
            buildEditFor (BD);
            showEdit (false);
        }
        stepView.repaint();
    };
    addAndMakeVisible (bassButton);

    juce::TextButton* varButtons[4] = { &varAButton, &varBButton, &varCButton, &varDButton };
    for (int i = 0; i < 4; ++i)
    {
        auto* b = varButtons[i];
        b->setClickingTogglesState (true);
        b->setColour (juce::TextButton::buttonOnColourId, Colors::orange);
        b->onClick = [this, i] { selectEditVar (i); };
        addAndMakeVisible (*b);
    }
    varAButton.setToggleState (true, juce::dontSendNotification);
    varLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (varLabel);

    abModeBox.addItem ("A", 1); abModeBox.addItem ("B", 2); abModeBox.addItem ("C", 3);
    abModeBox.addItem ("D", 4); abModeBox.addItem ("CYCLE", 5);
    abModeBox.onChange = [this]
    {
        const Sequencer::PlayMode modes[5] = { Sequencer::PlayMode::a, Sequencer::PlayMode::b,
                                               Sequencer::PlayMode::c, Sequencer::PlayMode::d,
                                               Sequencer::PlayMode::cycle };
        proc.getSequencer().setPlayMode (modes[juce::jlimit (0, 4, abModeBox.getSelectedId() - 1)]);
    };
    addAndMakeVisible (abModeBox);

    setupPresetBox (kitBox, PresetManager::kitNames(), "KIT");
    kitBox.onChange = [this] { handleKitBox(); };
    setupPresetBox (patternBox, PresetManager::patternNames(), "PATTERN");
    patternBox.onChange = [this] { handlePatternBox(); };
    addAndMakeVisible (kitBox); addAndMakeVisible (patternBox);   // boxes are self-labelled via placeholder

    tempoSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    tempoSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 44, 16);
    tempoSlider.setRange (20.0, 300.0, 1.0);
    tempoSlider.onValueChange = [this] { proc.getSequencer().setTempo (tempoSlider.getValue()); };
    tempoLabel.setFont (juce::FontOptions (12.5f, juce::Font::bold));
    tempoLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (tempoSlider); addAndMakeVisible (tempoLabel);

    swingSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    swingSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 44, 16);
    swingSlider.setRange (0.0, 0.75, 0.01);
    swingSlider.onValueChange = [this] { proc.getSequencer().setSwing ((float) swingSlider.getValue()); };
    swingLabel.setFont (juce::FontOptions (12.5f, juce::Font::bold));
    swingLabel.setJustificationType (juce::Justification::centred);
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
        for (int v = 0; v < Sequencer::numVars; ++v) proc.getSequencer().setLength (pat, v, L);
        stepView.repaint();
    };
    lenLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (lenBox); addAndMakeVisible (lenLabel);

    tripButton.setClickingTogglesState (true);
    tripButton.onClick = [this]
    {
        const bool trip = tripButton.getToggleState();
        const int pat = proc.getSequencer().getCurrentPattern();
        for (int v = 0; v < Sequencer::numVars; ++v) proc.getSequencer().setStepDiv (pat, v, trip ? (1.0f / 6.0f) : 0.25f);
        tripButton.setButtonText (trip ? "1/16T" : "1/16");
    };
    addAndMakeVisible (tripButton);

    // Time signature: sets the visible beat grouping (4/4 or 3/4) and the bar length.
    sigBox.addItem ("4/4", 1); sigBox.addItem ("3/4", 2);
    sigBox.setSelectedId (1, juce::dontSendNotification);
    sigBox.onChange = [this]
    {
        const bool threeFour = (sigBox.getSelectedId() == 2);
        const int group = threeFour ? 3 : 4;           // shade group size
        const int L = threeFour ? 12 : 16;             // steps per bar
        const int pat = proc.getSequencer().getCurrentPattern();
        for (int v = 0; v < Sequencer::numVars; ++v) proc.getSequencer().setLength (pat, v, L);
        stepView.setGrouping (group);
        stepView.repaint();
    };
    sigLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (sigBox); addAndMakeVisible (sigLabel);

    // Clear: wipe all drum programming in the current pattern (both A/B).
    clearButton.setColour (juce::TextButton::buttonColourId, Colors::red.withAlpha (0.85f));
    clearButton.setTooltip ("Clear all steps in the current pattern (A + B)");
    clearButton.onClick = [this]
    {
        proc.getSequencer().clearPattern (proc.getSequencer().getCurrentPattern());
        stepView.repaint();
    };
    addAndMakeVisible (clearButton);

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

    addChildComponent (fxPanel);
    fxTitle.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    fxTitle.setColour (juce::Label::textColourId, Colors::orange);
    addChildComponent (fxTitle);
    buildFx();

    addAndMakeVisible (stepView);
    stepView.setSelectedVoice (selectedVoice);
    stepView.onSelect = [this] (int v)
    {
        selectedVoice = v;
        bassButton.setToggleState (v == StepSequencerView::bassIndex, juce::dontSendNotification);
        if (v >= 0 && v < numVoices)              { buildEditFor (v); }
        else if (v == StepSequencerView::bassIndex) { buildBassEdit(); showEdit (true); }
    };
    stepView.onPreview = [this] (int v) { proc.previewVoice (v); };
    stepView.onPreviewBass = [this] (int note) { proc.previewBass (note); };

    syncTransport();
    showEdit (false);

    setResizable (true, true);
    setResizeLimits (1000, 560, 2400, 1500);   // header is control-dense; keep it from overlapping
    setSize (1180, 720);
}

TR808AudioProcessorEditor::~TR808AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void TR808AudioProcessorEditor::selectEditVar (int v)
{
    editVariation = juce::jlimit (0, Sequencer::numVars - 1, v);
    juce::TextButton* vb[4] = { &varAButton, &varBButton, &varCButton, &varDButton };
    for (int i = 0; i < 4; ++i)
        vb[i]->setToggleState (i == editVariation, juce::dontSendNotification);
    stepView.setEditVariation (editVariation);
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
            proc.previewVoice (vi);   // audition the instrument when selected
        });
        voiceColumns.add (col);
        performPanel.addAndMakeVisible (col);
    }
}

void TR808AudioProcessorEditor::buildEditFor (int voice)
{
    editControls.clear();
    editTitle.setText (juce::String (voiceSpecs()[(size_t) voice].name) + " - DEEP EDIT", juce::dontSendNotification);

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

    addKnob (macroId (voice, "revsend"), "RVB SEND");
    addKnob (macroId (voice, "dlysend"), "DLY SEND");

    auto* mt = new ParamToggle (proc.apvts, macroId (voice, "mute"), "MUTE", Colors::red);
    auto* st = new ParamToggle (proc.apvts, macroId (voice, "solo"), "SOLO", Colors::yellow);
    editControls.add (mt); editControls.add (st);
    editPanel.addAndMakeVisible (mt);
    editPanel.addAndMakeVisible (st);

    resized();
}

void TR808AudioProcessorEditor::buildBassEdit()
{
    editControls.clear();
    editTitle.setText ("BD BASS - EDIT", juce::dontSendNotification);
    auto add = [&] (const char* id, const juce::String& label)
    {
        auto* k = new tr808::ui::ParamKnob (proc.apvts, id, label);
        editControls.add (k);
        editPanel.addAndMakeVisible (k);
    };
    add (ParamIDs::bassLevel,   "LEVEL");
    add (ParamIDs::bassTone,    "TONE");
    add (ParamIDs::bassDecay,   "DECAY");
    add (ParamIDs::bassPunch,   "PUNCH");
    add (ParamIDs::bassDrive,   "DRIVE");
    add (ParamIDs::bassRevSend, "RVB SEND");
    add (ParamIDs::bassDlySend, "DLY SEND");
    resized();
}

void TR808AudioProcessorEditor::buildFx()
{
    fxControls.clear();
    auto add = [&] (const char* id, const juce::String& label)
    {
        auto* k = new tr808::ui::ParamKnob (proc.apvts, id, label);
        fxControls.add (k);
        fxPanel.addAndMakeVisible (k);
    };
    // Reverb (Lexicon-style) then ping-pong delay, then the melodic-bass sends.
    add (ParamIDs::revPredelay,  "PREDELAY");
    add (ParamIDs::revDecay,     "DECAY");
    add (ParamIDs::revBass,      "BASS");
    add (ParamIDs::revCrossover, "CROSS");
    add (ParamIDs::revDamp,      "DAMP");
    add (ParamIDs::revDepth,     "DEPTH");
    add (ParamIDs::revReturn,    "RVB RET");
    add (ParamIDs::dlyTime,      "DLY TIME");
    add (ParamIDs::dlyFeedback,  "FEEDBACK");
    add (ParamIDs::dlyTone,      "DLY TONE");
    add (ParamIDs::dlyReturn,    "DLY RET");
    add (ParamIDs::bassRevSend,  "BASS RVB");
    add (ParamIDs::bassDlySend,  "BASS DLY");
}

void TR808AudioProcessorEditor::showEdit (bool edit)
{
    editMode = edit;
    fxMode = false;
    performViewport.setVisible (! edit);
    editViewport.setVisible (edit);
    editTitle.setVisible (edit);
    fxPanel.setVisible (false);
    fxTitle.setVisible (false);
    fxButton.setToggleState (false, juce::dontSendNotification);
    viewButton.setButtonText (edit ? "PERF" : "EDIT");
    resized();
}

void TR808AudioProcessorEditor::showFx (bool fx)
{
    fxMode = fx;
    if (fx) editMode = false;
    performViewport.setVisible (! fx && ! editMode);
    editViewport.setVisible (! fx && editMode);
    editTitle.setVisible (! fx && editMode);
    fxPanel.setVisible (fx);
    fxTitle.setVisible (fx);
    fxButton.setToggleState (fx, juce::dontSendNotification);
    viewButton.setButtonText (editMode ? "PERF" : "EDIT");
    resized();
}

void TR808AudioProcessorEditor::setupPresetBox (juce::ComboBox& box, const juce::StringArray& factory,
                                                const juce::String& placeholder)
{
    box.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& n : factory) box.addItem (n, id++);
    box.addSeparator();
    box.addItem ("Random", 102);
    box.addItem ("Save", 103);          // updates the last Save As (disabled until then)
    box.addItem ("Save As...", 100);
    box.addItem ("Load...", 101);
    box.setItemEnabled (103, false);
    box.setTextWhenNothingSelected (placeholder);   // shows e.g. "KIT" / "PATTERN" until a preset is picked
}

void TR808AudioProcessorEditor::handleKitBox()
{
    const int id = kitBox.getSelectedId();
    if (id == 0) return;

    if (id == 100)
    {
        chooser = std::make_unique<juce::FileChooser> ("Save Kit As", PresetManager::presetsDir(), "*.xml");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f != juce::File())
                { lastKitFile = f.withFileExtension ("xml"); PresetManager::saveKit (proc.apvts, lastKitFile); kitBox.setItemEnabled (103, true); } });
    }
    else if (id == 103)                 // Save: update the last Save As
    {
        if (lastKitFile != juce::File()) PresetManager::saveKit (proc.apvts, lastKitFile);
    }
    else if (id == 101)
    {
        chooser = std::make_unique<juce::FileChooser> ("Load Kit", PresetManager::presetsDir(), "*.xml");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f.existsAsFile())
                { PresetManager::loadKit (proc.apvts, f); lastKitFile = f; kitBox.setItemEnabled (103, true); } });
    }
    else if (id == 102)
    {
        PresetManager::randomizeKit (proc.apvts);
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
        chooser = std::make_unique<juce::FileChooser> ("Save Pattern As", PresetManager::presetsDir(), "*.xml");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f != juce::File())
                { lastPatternFile = f.withFileExtension ("xml"); PresetManager::savePattern (proc.getSequencer(), lastPatternFile); patternBox.setItemEnabled (103, true); } });
    }
    else if (id == 103)                 // Save: update the last Save As
    {
        if (lastPatternFile != juce::File()) PresetManager::savePattern (proc.getSequencer(), lastPatternFile);
    }
    else if (id == 101)
    {
        chooser = std::make_unique<juce::FileChooser> ("Load Pattern", PresetManager::presetsDir(), "*.xml");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f.existsAsFile())
                { PresetManager::loadPattern (proc.getSequencer(), f); lastPatternFile = f; patternBox.setItemEnabled (103, true); } });
    }
    else if (id == 102)
    {
        PresetManager::randomizePattern (proc.getSequencer());
        syncTransport();
        stepView.repaint();
    }
    else
    {
        PresetManager::applyFactoryPattern (proc.getSequencer(), id - 1);
        stepView.repaint();
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
    abModeBox.setSelectedId ((int) pm + 1, juce::dontSendNotification);   // a..d=1..4, cycle=5

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
    abModeBox.setBounds (row1.removeFromLeft (74).reduced (2, 12));
    patLabel.setBounds (row1.removeFromLeft (26));
    patBox.setBounds (row1.removeFromLeft (48).reduced (2, 12));
    fxButton.setBounds (row1.removeFromLeft (46).reduced (2, 12));
    if (masterGainKnob)  masterGainKnob->setBounds (row1.removeFromRight (54));
    if (masterDriveKnob) masterDriveKnob->setBounds (row1.removeFromRight (54));
    if (accentKnob)      accentKnob->setBounds (row1.removeFromRight (54));
    if (multiOutToggle)  multiOutToggle->setBounds (row1.removeFromRight (52).reduced (2, 12));

    // row 2: presets + length/triplet + view/grid/variation
    // (KIT / PATTERN combos are self-labelled via their placeholder text)
    kitBox.setBounds (row2.removeFromLeft (118).reduced (2, 8));
    patternBox.setBounds (row2.removeFromLeft (134).reduced (2, 8));
    lenLabel.setBounds (row2.removeFromLeft (24));
    lenBox.setBounds (row2.removeFromLeft (46).reduced (2, 8));
    sigLabel.setBounds (row2.removeFromLeft (24));
    sigBox.setBounds (row2.removeFromLeft (56).reduced (2, 8));
    tripButton.setBounds (row2.removeFromLeft (50).reduced (2, 8));
    songButton.setBounds (row2.removeFromLeft (58).reduced (2, 8));
    clearButton.setBounds (row2.removeFromLeft (44).reduced (2, 8));
    chainEditor.setBounds (row2.removeFromLeft (72).reduced (2, 10));
    viewButton.setBounds (row2.removeFromRight (52).reduced (2, 8));
    gridButton.setBounds (row2.removeFromRight (52).reduced (2, 8));
    bassButton.setBounds (row2.removeFromRight (64).reduced (2, 8));
    varDButton.setBounds (row2.removeFromRight (28).reduced (2, 8));
    varCButton.setBounds (row2.removeFromRight (28).reduced (2, 8));
    varBButton.setBounds (row2.removeFromRight (28).reduced (2, 8));
    varAButton.setBounds (row2.removeFromRight (28).reduced (2, 8));
    varLabel.setBounds (row2.removeFromRight (34).reduced (0, 8));

    // --- step grid at the bottom ---
    auto stepArea = area.removeFromBottom (juce::jmax (170, area.getHeight() * 40 / 100));
    stepBounds = stepArea.reduced (6);
    stepView.setBounds (stepBounds);

    // --- middle: PERFORM or EDIT ---
    auto mid = area.reduced (6);

    performViewport.setBounds (mid);
    {
        const int colW = 120;
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
        const int panelW = juce::jmax (cw, editViewport.getMaximumVisibleWidth());
        const int cols = juce::jmax (1, panelW / cw);
        const int n = editControls.size();
        const int rows = (n + cols - 1) / juce::jmax (1, cols);
        editPanel.setSize (panelW, juce::jmax (ch, rows * ch));
        // Centre each row (including a partial last row) rather than left-aligning.
        for (int i = 0; i < n; ++i)
        {
            const int row = i / cols;
            const int colsInRow = juce::jmin (cols, n - row * cols);
            const int x0 = (panelW - colsInRow * cw) / 2;
            editControls[i]->setBounds (x0 + (i % cols) * cw, row * ch, cw - 4, ch - 4);
        }
    }

    // FX panel (reverb + delay): a simple grid of knobs over the mid area.
    {
        auto fxArea = mid;
        fxTitle.setBounds (fxArea.removeFromTop (20));
        fxPanel.setBounds (fxArea);
        const int cw = 92, ch = 96;
        const int cols = juce::jmax (1, fxArea.getWidth() / cw);
        for (int i = 0; i < fxControls.size(); ++i)
            fxControls[i]->setBounds ((i % cols) * cw + 4, (i / cols) * ch + 2, cw - 8, ch - 8);
    }
}
