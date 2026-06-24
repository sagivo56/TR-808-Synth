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
    // Let the mouse wheel scroll the (horizontally-scrolling) PERFORM panel
    // rather than adjust a knob under the cursor.
    for (auto* k : knobs) { k->setWheelEnabled (false); addAndMakeVisible (k); }

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

void TR808AudioProcessorEditor::ShadedPanel::paint (juce::Graphics& g)
{
    auto b = getLocalBounds();
    const int s1 = juce::jlimit (0, b.getHeight(), split1);
    const int s2 = juce::jlimit (s1, b.getHeight(), split2);
    g.setColour (juce::Colour (0xff2a2f3a));   // cool  = REVERB
    g.fillRoundedRectangle (b.removeFromTop (s1).toFloat().reduced (2.0f), 5.0f);
    g.setColour (juce::Colour (0xff3a2e26));   // warm  = DELAY
    g.fillRoundedRectangle (b.removeFromTop (s2 - s1).toFloat().reduced (2.0f), 5.0f);
    g.setColour (juce::Colour (0xff263a2c));   // green = DRIVE
    g.fillRoundedRectangle (b.toFloat().reduced (2.0f), 5.0f);
}

bool TR808AudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        const bool play = ! proc.getSequencer().isRunning();
        proc.getSequencer().setRunning (play);
        playButton.setToggleState (play, juce::dontSendNotification);
        return true;
    }
    return false;
}

void TR808AudioProcessorEditor::VoiceColumn::resized()
{
    auto r = getLocalBounds().reduced (2);
    selectButton.setBounds (r.removeFromTop (18));

    auto ms = r.removeFromBottom (18);
    mute->setBounds (ms.removeFromLeft (ms.getWidth() / 2).reduced (1));
    solo->setBounds (ms.reduced (1));

    // Two knobs per row (side by side) so the extra send knobs fit without a
    // very tall column. A lone knob on the last row (odd count) is centred.
    const int n = knobs.size();
    const int rows = (n + 1) / 2;
    const int kh = rows > 0 ? r.getHeight() / rows : r.getHeight();
    const int colW = r.getWidth() / 2;
    for (int i = 0; i < n; ++i)
    {
        const bool lastAlone = (i == n - 1) && (n % 2 == 1);   // odd -> last knob alone in its row
        const int x = lastAlone ? r.getX() + (r.getWidth() - colW) / 2
                                : r.getX() + (i % 2) * colW;
        knobs[i]->setBounds (x, r.getY() + (i / 2) * kh, colW, kh);
    }
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

    // --- mode tabs: FX / BD BASS / MAIN (global sections above the instruments) ---
    fxButton.setClickingTogglesState (true);
    fxButton.setColour (juce::TextButton::buttonOnColourId, Colors::orange);
    fxButton.onClick = [this] { setEditorMode (Mode::fx); };
    addAndMakeVisible (fxButton);

    bassButton.setClickingTogglesState (true);
    bassButton.setColour (juce::TextButton::buttonOnColourId, Colors::orange);
    bassButton.onClick = [this] { setEditorMode (Mode::bass); };
    addAndMakeVisible (bassButton);

    mainButton.setClickingTogglesState (true);
    mainButton.setColour (juce::TextButton::buttonOnColourId, Colors::orange);
    mainButton.onClick = [this] { setEditorMode (Mode::main); };
    addAndMakeVisible (mainButton);

    // --- instrument toolbar: pick one drum -> its params + its pads ---
    for (int v = 0; v < numVoices; ++v)
    {
        auto* b = new juce::TextButton (juce::String (voiceSpecs()[(size_t) v].prefix).toUpperCase());
        b->setClickingTogglesState (true);
        b->setColour (juce::TextButton::buttonOnColourId, Colors::orange);
        b->onClick = [this, v] { selectDrum (v); };
        instButtons.add (b);
        addAndMakeVisible (b);
    }
    stepView.setShowSelector (false);   // the toolbar above replaces the built-in selector
    stepView.setMode (StepSequencerView::Mode::authentic);

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

    // Copy the currently-edited variation onto another (A->B etc.).
    copyBox.setTextWhenNothingSelected ("COPY");
    copyBox.addItem ("to A", 1); copyBox.addItem ("to B", 2);
    copyBox.addItem ("to C", 3); copyBox.addItem ("to D", 4);
    copyBox.onChange = [this]
    {
        const int dst = copyBox.getSelectedId() - 1;
        if (dst >= 0 && dst != editVariation)
        {
            proc.getSequencer().copyVariation (proc.getSequencer().getCurrentPattern(), editVariation, dst);
            stepView.repaint();
        }
        copyBox.setSelectedId (0, juce::dontSendNotification);
    };
    addAndMakeVisible (copyBox);

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
    lenBox.setTextWhenNothingSelected ("LEN");
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
    sigBox.setTextWhenNothingSelected ("SIG");
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

    editViewport.setViewedComponent (&editPanel, false);
    editViewport.setScrollBarsShown (true, false);
    addChildComponent (editViewport);
    editTitle.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    editTitle.setColour (juce::Label::textColourId, Colors::orange);
    addChildComponent (editTitle);

    addChildComponent (fxPanel);
    fxTitle.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    fxTitle.setColour (juce::Label::textColourId, Colors::orange);
    addChildComponent (fxTitle);
    buildFx();

    addChildComponent (mainPanel);
    mainTitle.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    mainTitle.setColour (juce::Label::textColourId, Colors::orange);
    addChildComponent (mainTitle);
    buildMain();

    addAndMakeVisible (stepView);
    stepView.onPreviewBass = [this] (int note) { proc.previewBass (note); };

    syncTransport();
    selectDrum (tr808::BD);                 // start on the BD instrument view

    setResizable (true, true);
    setWantsKeyboardFocus (true);               // so SPACE toggles transport
    setResizeLimits (380, 480, 2400, 1500);   // small min => compact/mobile layout kicks in under 860px
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

void TR808AudioProcessorEditor::buildPerform() {}   // PERFORM all-columns view retired (one-instrument focus now)

void TR808AudioProcessorEditor::selectDrum (int voice)
{
    if (voice < 0 || voice >= numVoices) return;
    selectedDrum = voice;
    mode = Mode::drum;
    buildEditFor (voice);
    stepView.setSelectedVoice (voice);
    proc.previewVoice (voice);          // audition it
    refreshModeUI();
}

void TR808AudioProcessorEditor::setEditorMode (Mode m)
{
    mode = m;
    if (m == Mode::bass) { buildBassEdit(); stepView.setSelectedVoice (StepSequencerView::bassIndex); }
    else                 { stepView.setSelectedVoice (selectedDrum); }
    if (m == Mode::drum) buildEditFor (selectedDrum);
    refreshModeUI();
}

void TR808AudioProcessorEditor::refreshModeUI()
{
    const bool drum = (mode == Mode::drum), fx = (mode == Mode::fx),
               bass = (mode == Mode::bass), main = (mode == Mode::main);

    editViewport.setVisible (drum || bass);   // editPanel holds instrument or bass params
    editTitle.setVisible   (drum || bass);
    fxPanel.setVisible (fx);   fxTitle.setVisible (fx);
    mainPanel.setVisible (main); mainTitle.setVisible (main);

    fxButton.setToggleState   (fx,   juce::dontSendNotification);
    bassButton.setToggleState (bass, juce::dontSendNotification);
    mainButton.setToggleState (main, juce::dontSendNotification);
    for (int i = 0; i < instButtons.size(); ++i)
        instButtons[i]->setToggleState (drum && i == selectedDrum, juce::dontSendNotification);

    resized();
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

    auto* duck = new tr808::ui::ParamToggle (proc.apvts, ParamIDs::bassDuckBd, "DUCK BD", Colors::orange);
    editControls.add (duck);
    editPanel.addAndMakeVisible (duck);
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
    // Reverb group, then delay group (visually separated in resized()).
    add (ParamIDs::revPredelay,  "PREDELAY");
    add (ParamIDs::revDecay,     "DECAY");
    add (ParamIDs::revBass,      "BASS");
    add (ParamIDs::revCrossover, "CROSS");
    add (ParamIDs::revDamp,      "DAMP");
    add (ParamIDs::revDepth,     "DEPTH");
    add (ParamIDs::revReturn,    "RVB RET");
    fxDelayStart = fxControls.size();
    add (ParamIDs::dlyTime,      "DLY TIME");
    add (ParamIDs::dlyFeedback,  "FEEDBACK");
    add (ParamIDs::dlyTone,      "DLY TONE");
    add (ParamIDs::dlyReturn,    "DLY RET");
    fxDriveStart = fxControls.size();
    add (ParamIDs::masterDrive,  "DRIVE");
    add (ParamIDs::masterGain,   "OUTPUT");

    auto styleLabel = [] (juce::Label& l)
    {
        l.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        l.setColour (juce::Label::textColourId, Colors::orange);
        l.setJustificationType (juce::Justification::centredLeft);
    };
    styleLabel (fxRevLabel); styleLabel (fxDlyLabel); styleLabel (fxDrvLabel);
    fxPanel.addAndMakeVisible (fxRevLabel);
    fxPanel.addAndMakeVisible (fxDlyLabel);
    fxPanel.addAndMakeVisible (fxDrvLabel);
}

void TR808AudioProcessorEditor::buildMain()
{
    // Re-home the master controls (created in the ctor) into the MAIN tab panel.
    if (masterGainKnob)  mainPanel.addAndMakeVisible (*masterGainKnob);
    if (masterDriveKnob) mainPanel.addAndMakeVisible (*masterDriveKnob);
    if (accentKnob)      mainPanel.addAndMakeVisible (*accentKnob);
    if (multiOutToggle)  mainPanel.addAndMakeVisible (*multiOutToggle);
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
    g.fillRect (0, 0, getWidth(), headerH);
    g.setColour (Colors::orange); g.fillRect (0, headerH - 4, getWidth(), 3);
    g.setColour (Colors::cream);  g.fillRect (0, headerH - 1, getWidth(), 1);

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
    titleLabel.setVisible (false);   // brand lives in the OS title bar now

    // ---- header: transport+presets / sequencer / instrument toolbar ----
    const int hpad = 6, r1h = 34, r2h = 32, r3h = 36;
    headerH = hpad + r1h + 2 + r2h + 2 + r3h + hpad;
    auto header = area.removeFromTop (headerH).reduced (hpad, hpad);
    auto row1 = header.removeFromTop (r1h);
    header.removeFromTop (2);
    auto row2 = header.removeFromTop (r2h);
    header.removeFromTop (2);
    auto row3 = header;   // instrument toolbar

    // row 1: transport + presets, with the 3 tabs on the right
    playButton.setBounds (row1.removeFromLeft (56).reduced (1, 2));
    { auto t = row1.removeFromLeft (152); tempoLabel.setBounds (t.removeFromLeft (46)); tempoSlider.setBounds (t.reduced (0, 4)); }
    { auto s = row1.removeFromLeft (152); swingLabel.setBounds (s.removeFromLeft (48)); swingSlider.setBounds (s.reduced (0, 4)); }
    kitBox.setBounds (row1.removeFromLeft (116).reduced (2, 1));
    patternBox.setBounds (row1.removeFromLeft (128).reduced (2, 1));
    mainButton.setBounds (row1.removeFromRight (56).reduced (2, 1));
    bassButton.setBounds (row1.removeFromRight (72).reduced (2, 1));
    fxButton.setBounds   (row1.removeFromRight (50).reduced (2, 1));

    // row 2: sequencer controls
    patLabel.setBounds (row2.removeFromLeft (26));
    patBox.setBounds (row2.removeFromLeft (46).reduced (2, 1));
    abModeBox.setBounds (row2.removeFromLeft (72).reduced (2, 1));
    varLabel.setBounds (row2.removeFromLeft (34));
    varAButton.setBounds (row2.removeFromLeft (28).reduced (1, 1));
    varBButton.setBounds (row2.removeFromLeft (28).reduced (1, 1));
    varCButton.setBounds (row2.removeFromLeft (28).reduced (1, 1));
    varDButton.setBounds (row2.removeFromLeft (28).reduced (1, 1));
    copyBox.setBounds (row2.removeFromLeft (74).reduced (2, 1));
    lenLabel.setBounds (row2.removeFromLeft (24));
    lenBox.setBounds (row2.removeFromLeft (58).reduced (2, 1));
    sigLabel.setBounds (row2.removeFromLeft (24));
    sigBox.setBounds (row2.removeFromLeft (62).reduced (2, 1));
    tripButton.setBounds (row2.removeFromLeft (50).reduced (2, 1));
    songButton.setBounds (row2.removeFromLeft (54).reduced (2, 1));
    clearButton.setBounds (row2.removeFromLeft (42).reduced (2, 1));
    chainEditor.setBounds (row2.removeFromLeft (60).reduced (2, 2));

    // row 3: instrument toolbar (equal share of the width)
    {
        const int n = instButtons.size();
        const float bw = n > 0 ? row3.getWidth() / (float) n : (float) row3.getWidth();
        for (int i = 0; i < n; ++i)
            instButtons[i]->setBounds ((int) (row3.getX() + i * bw), row3.getY(),
                                       (int) bw - 2, row3.getHeight());
    }

    // ---- body: active params panel (top) + step pads (bottom) ----
    auto stepArea = area.removeFromBottom (juce::jmax (160, area.getHeight() * 50 / 100));
    stepBounds = stepArea.reduced (6);
    stepView.setBounds (stepBounds);

    auto mid = area.reduced (6);

    // Instrument / BD-Bass params (editPanel inside its scroll viewport)
    {
        auto editArea = mid;
        editTitle.setBounds (editArea.removeFromTop (20));
        editViewport.setBounds (editArea);
        const int cw = 94, ch = 98;
        const int panelW = juce::jmax (cw, editViewport.getMaximumVisibleWidth());
        const int cols = juce::jmax (1, panelW / cw);
        const int n = editControls.size();
        const int rows = (n + cols - 1) / juce::jmax (1, cols);
        editPanel.setSize (panelW, juce::jmax (ch, rows * ch));
        for (int i = 0; i < n; ++i)
        {
            const int row = i / cols;
            const int colsInRow = juce::jmin (cols, n - row * cols);
            const int x0 = (panelW - colsInRow * cw) / 2;
            editControls[i]->setBounds (x0 + (i % cols) * cw, row * ch, cw - 4, ch - 4);
        }
    }

    // MAIN panel: master knobs centred in a row
    {
        auto mArea = mid;
        mainTitle.setBounds (mArea.removeFromTop (20));
        mainPanel.setBounds (mArea);
        const int kw = 100, kh = juce::jmin (mArea.getHeight() - 8, 130);
        const int toggleW = 92;
        const int total = 3 * kw + toggleW + 16;
        int x = juce::jmax (8, (mArea.getWidth() - total) / 2), y = 6;
        juce::Component* mk[] = { masterGainKnob.get(), masterDriveKnob.get(), accentKnob.get() };
        for (auto* k : mk) if (k) { k->setBounds (x, y, kw - 6, kh); x += kw; }
        if (multiOutToggle) multiOutToggle->setBounds (x + 8, y + kh / 2 - 16, toggleW, 30);
    }

    // FX panel: reverb / delay / drive bands.
    {
        auto fxArea = mid;
        fxTitle.setBounds (fxArea.removeFromTop (20));
        fxPanel.setBounds (fxArea);
        const int cw = 92, ch = 90, labelH = 18;
        const int bandH = labelH + ch;
        const int panelW = fxArea.getWidth();
        fxPanel.split1 = bandH;                  // reverb / delay boundary
        fxPanel.split2 = 2 * bandH;              // delay / drive boundary
        auto rowCentred = [&] (int from, int to, int y)
        {
            const int count = to - from;
            const int x0 = juce::jmax (0, (panelW - count * cw) / 2);
            for (int i = from; i < to; ++i)
                fxControls[i]->setBounds (x0 + (i - from) * cw + 4, y + 2, cw - 8, ch - 8);
        };
        const int n = fxControls.size();
        fxRevLabel.setBounds (8, 0, panelW - 16, labelH);
        rowCentred (0, fxDelayStart, labelH);
        fxDlyLabel.setBounds (8, bandH, panelW - 16, labelH);
        rowCentred (fxDelayStart, fxDriveStart, bandH + labelH);
        fxDrvLabel.setBounds (8, 2 * bandH, panelW - 16, labelH);
        rowCentred (fxDriveStart, n, 2 * bandH + labelH);
    }
}
