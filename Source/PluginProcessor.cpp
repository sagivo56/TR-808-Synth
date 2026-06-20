#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    // Nothing to allocate yet — voices/sequencer get prepared here from M2 on.
    juce::ignoreUnused (sampleRate, samplesPerBlock);
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
    juce::ignoreUnused (midiMessages);

    // M0: render silence. Clearing the whole buffer also wipes any output
    // channels the host provided beyond what we use.
    buffer.clear();
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
