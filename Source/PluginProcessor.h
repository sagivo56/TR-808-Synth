#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <vector>
#include <utility>

#include "params/ParameterLayout.h"
#include "engine/VoiceManager.h"
#include "engine/Sequencer.h"

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

    tr808::Sequencer& getSequencer() noexcept { return sequencer; }

    // Audition a voice from the UI (one-shot). Thread-safe: the audio thread
    // drains the request next block.
    void previewVoice (int voiceIndex) noexcept { previewRequested.store (voiceIndex); }
    void previewBass  (int midiNote)   noexcept { previewBassNote.store (midiNote); }

    static BusesProperties makeBusesProperties();

private:
    void updateMacrosFromApvts();
    void updateDeepFromApvts();
    void updateMixerFromApvts();
    void updateFxFromApvts();

    tr808::VoiceManager voiceManager;
    tr808::Sequencer    sequencer;
    tr808::Mixer        mixer;

    // Reusable, pre-reserved per-block event buffer (no audio-thread allocation).
    std::vector<tr808::TriggerEvent> eventBuffer;

    // UI preview requests (drained each block): a drum voice index, or a bass
    // MIDI note. Both -1 when idle.
    std::atomic<int> previewRequested { -1 };
    std::atomic<int> previewBassNote { -1 };

    juce::LinearSmoothedValue<float> masterGain;
    std::atomic<float>* masterGainParam = nullptr;

    // Cached APVTS pointers per voice (nullptr where a voice lacks that macro).
    std::array<std::atomic<float>*, tr808::numVoices> levelP {}, toneP {}, decayP {}, snappyP {}, tuneP {};

    // Deep params: APVTS atomic -> destination float inside the voice.
    std::vector<std::pair<std::atomic<float>*, float*>> deepWiring;

    // Mixer params.
    std::array<std::atomic<float>*, tr808::numVoices> panP {}, muteP {}, soloP {};
    std::atomic<float>* masterDriveParam = nullptr;
    std::atomic<float>* multiOutParam = nullptr;
    std::atomic<float>* accentLevelParam = nullptr;
    std::array<float*, tr808::numVoices> auxPtr {};   // per-block aux channel pointers

    // Parallel FX: per-voice sends + global reverb/delay params.
    std::array<std::atomic<float>*, tr808::numVoices> revSendP {}, dlySendP {};
    std::atomic<float>* bassRevSendP = nullptr;
    std::atomic<float>* bassDlySendP = nullptr;
    std::atomic<float>* revPredelayP = nullptr; std::atomic<float>* revDecayP = nullptr;
    std::atomic<float>* revBassP = nullptr;     std::atomic<float>* revCrossP = nullptr;
    std::atomic<float>* revDampP = nullptr;     std::atomic<float>* revDepthP = nullptr;
    std::atomic<float>* revReturnP = nullptr;
    std::atomic<float>* dlyTimeP = nullptr;     std::atomic<float>* dlyFbP = nullptr;
    std::atomic<float>* dlyToneP = nullptr;     std::atomic<float>* dlyReturnP = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TR808AudioProcessor)
};
