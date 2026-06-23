#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "params/ParameterIDs.h"

#include <algorithm>

//==============================================================================
juce::AudioProcessor::BusesProperties TR808AudioProcessor::makeBusesProperties()
{
    // Main stereo master + one mono aux ("multi-out") bus per voice, disabled by
    // default so the default layout is plain stereo. Hosts enable the aux buses
    // for individual outputs.
    auto buses = BusesProperties().withOutput ("Master", juce::AudioChannelSet::stereo(), true);
    for (int i = 0; i < tr808::numVoices; ++i)
        buses = buses.withOutput (tr808::voiceSpecs()[(size_t) i].name, juce::AudioChannelSet::mono(), false);
    return buses;
}

TR808AudioProcessor::TR808AudioProcessor()
    : AudioProcessor (makeBusesProperties()),
      apvts (*this, nullptr, "PARAMETERS", params::createParameterLayout())
{
}

//==============================================================================
void TR808AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voiceManager.prepare (sampleRate, samplesPerBlock);
    sequencer.prepare (sampleRate);
    mixer.prepare (sampleRate, samplesPerBlock);
    eventBuffer.reserve (1024);

    masterDriveParam = apvts.getRawParameterValue (ParamIDs::masterDrive);
    multiOutParam    = apvts.getRawParameterValue (ParamIDs::multiOut);
    accentLevelParam = apvts.getRawParameterValue (ParamIDs::accentLevel);
    for (int i = 0; i < tr808::numVoices; ++i)
    {
        panP[(size_t) i]  = apvts.getRawParameterValue (juce::String (tr808::macroId (i, "pan")));
        muteP[(size_t) i] = apvts.getRawParameterValue (juce::String (tr808::macroId (i, "mute")));
        soloP[(size_t) i] = apvts.getRawParameterValue (juce::String (tr808::macroId (i, "solo")));
    }

    masterGainParam = apvts.getRawParameterValue (ParamIDs::masterGain);
    masterGain.reset (sampleRate, 0.02);
    masterGain.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (masterGainParam != nullptr ? masterGainParam->load() : 0.0f));

    // Cache the APVTS atomics for each voice's Macro params.
    for (int i = 0; i < tr808::numVoices; ++i)
    {
        const auto& s = tr808::voiceSpecs()[(size_t) i];
        levelP[(size_t) i]  =            apvts.getRawParameterValue (juce::String (tr808::macroId (i, "level")));
        toneP[(size_t) i]   = s.tone   ? apvts.getRawParameterValue (juce::String (tr808::macroId (i, "tone")))   : nullptr;
        decayP[(size_t) i]  = s.decay  ? apvts.getRawParameterValue (juce::String (tr808::macroId (i, "decay")))  : nullptr;
        snappyP[(size_t) i] = s.snappy ? apvts.getRawParameterValue (juce::String (tr808::macroId (i, "snappy"))) : nullptr;
        tuneP[(size_t) i]   = s.tune   ? apvts.getRawParameterValue (juce::String (tr808::macroId (i, "tune")))   : nullptr;
        revSendP[(size_t) i] = apvts.getRawParameterValue (juce::String (tr808::macroId (i, "revsend")));
        dlySendP[(size_t) i] = apvts.getRawParameterValue (juce::String (tr808::macroId (i, "dlysend")));
    }

    bassRevSendP = apvts.getRawParameterValue (ParamIDs::bassRevSend);
    bassDlySendP = apvts.getRawParameterValue (ParamIDs::bassDlySend);
    revPredelayP = apvts.getRawParameterValue (ParamIDs::revPredelay);
    revDecayP    = apvts.getRawParameterValue (ParamIDs::revDecay);
    revBassP     = apvts.getRawParameterValue (ParamIDs::revBass);
    revCrossP    = apvts.getRawParameterValue (ParamIDs::revCrossover);
    revDampP     = apvts.getRawParameterValue (ParamIDs::revDamp);
    revDepthP    = apvts.getRawParameterValue (ParamIDs::revDepth);
    revReturnP   = apvts.getRawParameterValue (ParamIDs::revReturn);
    dlyTimeP     = apvts.getRawParameterValue (ParamIDs::dlyTime);
    dlyFbP       = apvts.getRawParameterValue (ParamIDs::dlyFeedback);
    dlyToneP     = apvts.getRawParameterValue (ParamIDs::dlyTone);
    dlyReturnP   = apvts.getRawParameterValue (ParamIDs::dlyReturn);

    // Pair every voice's Deep param with its APVTS atomic (built once here).
    deepWiring.clear();
    for (int i = 0; i < tr808::numVoices; ++i)
        if (auto* v = voiceManager.voice (i))
            for (const auto& ref : v->deepRefs())
                if (auto* atom = apvts.getRawParameterValue (juce::String (tr808::macroId (i, ref.suffix.c_str()))))
                    if (ref.ptr != nullptr)
                        deepWiring.push_back ({ atom, ref.ptr });
}

void TR808AudioProcessor::updateDeepFromApvts()
{
    for (auto& w : deepWiring)
        *w.second = w.first->load();
}

void TR808AudioProcessor::updateMixerFromApvts()
{
    for (int i = 0; i < tr808::numVoices; ++i)
    {
        mixer.setPan (i, panP[(size_t) i] != nullptr ? panP[(size_t) i]->load() : 0.0f);

        const bool m = muteP[(size_t) i] != nullptr && muteP[(size_t) i]->load() > 0.5f;
        const bool s = soloP[(size_t) i] != nullptr && soloP[(size_t) i]->load() > 0.5f;
        mixer.setMute (i, m);
        mixer.setSolo (i, s);

        // Single source of truth: also keep the sequencer from firing them.
        sequencer.setMute (i, m);
        sequencer.setSolo (i, s);
    }
    mixer.setMasterDrive (masterDriveParam != nullptr ? masterDriveParam->load() : 1.0f);
    voiceManager.setAccentAmount (accentLevelParam != nullptr ? accentLevelParam->load() : 1.5f);
    voiceManager.setBassGate (sequencer.getBassGate());
}

void TR808AudioProcessor::updateFxFromApvts()
{
    auto get = [] (std::atomic<float>* p, float d) { return p != nullptr ? p->load() : d; };

    for (int i = 0; i < tr808::numVoices; ++i)
    {
        voiceManager.setReverbSend (i, get (revSendP[(size_t) i], 0.0f));
        voiceManager.setDelaySend  (i, get (dlySendP[(size_t) i], 0.0f));
    }
    voiceManager.setReverbSend (tr808::VoiceManager::bassVoiceIndex, get (bassRevSendP, 0.0f));
    voiceManager.setDelaySend  (tr808::VoiceManager::bassVoiceIndex, get (bassDlySendP, 0.0f));

    auto& rev = voiceManager.reverb();
    rev.setPredelay  (get (revPredelayP, 15.0f));
    rev.setDecay     (get (revDecayP, 2.2f));
    rev.setBassMult  (get (revBassP, 1.4f));
    rev.setCrossover (get (revCrossP, 500.0f));
    rev.setDamping   (get (revDampP, 0.35f));
    rev.setDepth     (get (revDepthP, 0.30f));
    voiceManager.setReverbReturn (get (revReturnP, 0.9f));

    auto& dly = voiceManager.delay();
    dly.setTime     (get (dlyTimeP, 350.0f));
    dly.setFeedback (get (dlyFbP, 0.35f));
    dly.setTone     (get (dlyToneP, 0.6f));
    voiceManager.setDelayReturn (get (dlyReturnP, 0.9f));
}

void TR808AudioProcessor::updateMacrosFromApvts()
{
    for (int i = 0; i < tr808::numVoices; ++i)
    {
        auto& m = voiceManager.macros[(size_t) i];
        m.level  = levelP[(size_t) i]  != nullptr ? levelP[(size_t) i]->load()  : 0.8f;
        m.tone   = toneP[(size_t) i]   != nullptr ? toneP[(size_t) i]->load()   : 0.5f;
        m.decay  = decayP[(size_t) i]  != nullptr ? decayP[(size_t) i]->load()  : 0.5f;
        m.snappy = snappyP[(size_t) i] != nullptr ? snappyP[(size_t) i]->load() : 0.5f;
        m.tune   = tuneP[(size_t) i]   != nullptr ? tuneP[(size_t) i]->load()   : 0.5f;
    }
}

void TR808AudioProcessor::releaseResources()
{
}

bool TR808AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Synthesiser: no audio input.
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;

    // Main (master) bus: mono or stereo.
    const auto main = layouts.getMainOutputChannelSet();
    if (main != juce::AudioChannelSet::mono() && main != juce::AudioChannelSet::stereo())
        return false;

    // Per-voice aux buses: each mono or disabled — so the host can run plain
    // stereo (all aux disabled = fallback) or full multi-out.
    for (int b = 1; b < layouts.outputBuses.size(); ++b)
    {
        const auto set = layouts.outputBuses.getReference (b);
        if (set != juce::AudioChannelSet::disabled() && set != juce::AudioChannelSet::mono())
            return false;
    }
    return true;
}

void TR808AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    updateMacrosFromApvts();
    updateDeepFromApvts();
    updateMixerFromApvts();
    updateFxFromApvts();

    // Transport from the host (falls back to the sequencer's internal clock).
    tr808::Sequencer::TransportInfo transport;
    transport.sampleRate = getSampleRate();
    transport.numSamples = buffer.getNumSamples();
    if (auto* ph = getPlayHead())
        if (const auto pos = ph->getPosition())
        {
            transport.hostPlaying = pos->getIsPlaying();
            if (const auto bpm = pos->getBpm())         transport.bpm         = *bpm;
            if (const auto ppq = pos->getPpqPosition()) transport.ppqPosition = *ppq;
        }

    // Sequencer events for this block, then merge in any incoming MIDI note-ons.
    sequencer.process (transport, eventBuffer);
    for (const auto meta : midiMessages)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
        {
            const int idx = tr808::gmNoteToVoice (msg.getNoteNumber());
            if (idx >= 0)
                eventBuffer.push_back ({ meta.samplePosition, idx, msg.getFloatVelocity(),
                                         msg.getVelocity() >= tr808::VoiceManager::accentVelocity });
        }
    }
    // UI preview: trigger the requested voice once at the top of this block.
    const int pv = previewRequested.exchange (-1);
    if (pv >= 0 && pv < tr808::numVoices)
        eventBuffer.push_back ({ 0, pv, 1.0f, false });

    const int pb = previewBassNote.exchange (-1);
    if (pb >= 0)
    {
        const float hz = 440.0f * std::pow (2.0f, (float) (pb - 69) / 12.0f);
        eventBuffer.push_back ({ 0, tr808::VoiceManager::bassVoiceIndex, 1.0f, false, hz });
    }

    std::sort (eventBuffer.begin(), eventBuffer.end(),
               [] (const auto& a, const auto& b) { return a.samplePos < b.samplePos; });

    // Routing: clear everything, then render the master stereo bus plus any
    // active per-voice aux (multi-out) sends.
    buffer.clear();

    const bool multiOut = (multiOutParam != nullptr && multiOutParam->load() > 0.5f);
    for (int v = 0; v < tr808::numVoices; ++v)
    {
        const int busIdx = v + 1;
        auxPtr[(size_t) v] = (multiOut && busIdx < getBusCount (false) && getChannelCountOfBus (false, busIdx) > 0)
                           ? getBusBuffer (buffer, false, busIdx).getWritePointer (0)
                           : nullptr;
    }

    auto mainBus = getBusBuffer (buffer, false, 0);
    voiceManager.renderEvents (mainBus, eventBuffer, mixer, auxPtr.data());

    mixer.processMaster (mainBus);

    if (masterGainParam != nullptr)
        masterGain.setTargetValue (juce::Decibels::decibelsToGain (masterGainParam->load()));
    masterGain.applyGain (mainBus, mainBus.getNumSamples());
}

//==============================================================================
juce::AudioProcessorEditor* TR808AudioProcessor::createEditor()
{
    return new TR808AudioProcessorEditor (*this);
}

//==============================================================================
void TR808AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Wrap params + sequencer so both are saved together.
    juce::ValueTree root ("TR808STATE");
    root.appendChild (apvts.copyState(), nullptr);
    root.appendChild (sequencer.toValueTree(), nullptr);

    if (auto xml = root.createXml())
        copyXmlToBinary (*xml, destData);
}

void TR808AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr)
        return;

    const auto tree = juce::ValueTree::fromXml (*xml);
    if (! tree.isValid())
        return;

    if (tree.hasType (apvts.state.getType()))           // legacy: bare PARAMETERS tree
    {
        apvts.replaceState (tree);
        return;
    }

    if (const auto params = tree.getChildWithName (apvts.state.getType()); params.isValid())
        apvts.replaceState (params);
    if (const auto seq = tree.getChildWithName ("SEQUENCER"); seq.isValid())
        sequencer.fromValueTree (seq);
}

//==============================================================================
// This creates new instances of the plugin — required by JUCE.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TR808AudioProcessor();
}
