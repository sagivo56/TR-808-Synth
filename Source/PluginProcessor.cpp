#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "params/ParameterIDs.h"

#include <algorithm>

//==============================================================================
TR808AudioProcessor::TR808AudioProcessor()
    : AudioProcessor (BusesProperties()
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", params::createParameterLayout())
{
}

//==============================================================================
void TR808AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voiceManager.prepare (sampleRate, samplesPerBlock);
    sequencer.prepare (sampleRate);
    eventBuffer.reserve (1024);

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
    }

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
    // Synthesiser: no audio input. The main output may be mono or stereo (M0);
    // the selectable 16-channel multi-out layout is added in M5.
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;

    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono()
        || out == juce::AudioChannelSet::stereo();
}

void TR808AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    updateMacrosFromApvts();
    updateDeepFromApvts();

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
    std::sort (eventBuffer.begin(), eventBuffer.end(),
               [] (const auto& a, const auto& b) { return a.samplePos < b.samplePos; });

    voiceManager.renderEvents (buffer, eventBuffer);

    if (masterGainParam != nullptr)
        masterGain.setTargetValue (juce::Decibels::decibelsToGain (masterGainParam->load()));
    masterGain.applyGain (buffer, buffer.getNumSamples());
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
