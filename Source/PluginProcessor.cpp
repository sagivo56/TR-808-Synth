#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "params/ParameterIDs.h"

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

    // The VoiceManager clears the buffer, then renders the voices with
    // sample-accurate triggers driven by the incoming MIDI.
    voiceManager.process (buffer, midiMessages);

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
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void TR808AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
// This creates new instances of the plugin — required by JUCE.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TR808AudioProcessor();
}
