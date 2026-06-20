#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>

#include "params/ParameterLayout.h"
#include "engine/VoiceManager.h"

//==============================================================================
// TR-808-style drum synthesiser — top-level AudioProcessor.
//
// M0 scaffold: holds the APVTS, accepts MIDI, declares a stereo output bus and
// renders silence. The voice engine, sequencer, mixer and multi-out routing are
// layered on in later milestones.
//==============================================================================
class TR808AudioProcessor : public juce::AudioProcessor
{
public:
    TR808AudioProcessor();
    ~TR808AudioProcessor() override = default;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;

private:
    void updateMacrosFromApvts();

    tr808::VoiceManager voiceManager;

    juce::LinearSmoothedValue<float> masterGain;
    std::atomic<float>* masterGainParam = nullptr;

    // Cached APVTS pointers per voice (nullptr where a voice lacks that macro).
    std::array<std::atomic<float>*, tr808::numVoices> levelP {}, toneP {}, decayP {}, snappyP {}, tuneP {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TR808AudioProcessor)
};
