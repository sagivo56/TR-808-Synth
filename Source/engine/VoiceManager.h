#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <memory>
#include <vector>

#include "VoiceDefs.h"
#include "Mixer.h"
#include "../voices/Voice.h"

namespace tr808
{
// A single voice trigger at a sample offset within the current block. Built
// from incoming MIDI and/or the Sequencer, merged and rendered sample-accurately.
struct TriggerEvent
{
    int   samplePos;
    int   voiceIndex;
    float velocity;
    bool  accent;
};

//==============================================================================
// Owns the 16 voices, maps GM notes to them, renders with sample-accurate
// triggers (the block is split at each event), sums the mono voices to stereo,
// and enforces the hi-hat choke group (OH/CH are mutually exclusive).
//==============================================================================
class VoiceManager
{
public:
    VoiceManager();

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    // Convenience MIDI-only path (used by tests): builds events from the MIDI
    // buffer and renders them.
    void process (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);

    // Core render: 'events' must be sorted by samplePos. Clears the main stereo
    // buffer (and any aux channels), splits at each event, triggers, renders
    // each voice and routes it through the Mixer — panned/gated to the master,
    // and (if a channel is provided) the clean mono voice to its aux send.
    // 'auxChannels' is numVoices pointers (nullptr where that send is inactive).
    void renderEvents (juce::AudioBuffer<float>& mainBuffer,
                       const std::vector<TriggerEvent>& events,
                       Mixer& mixer,
                       float* const* auxChannels);

    void noteOn (int voiceIndex, float velocity, bool accent);
    bool isVoiceActive (int voiceIndex) const;

    void setAccentAmount (float a) noexcept { accentAmount = juce::jmax (1.0f, a); }

    voices::Voice* voice (int voiceIndex) noexcept
    {
        return (voiceIndex >= 0 && voiceIndex < numVoices) ? voiceArray[(size_t) voiceIndex].get() : nullptr;
    }

    // Per-voice Macro values; the processor refreshes these from the APVTS each
    // block. They are latched into a voice when it is triggered.
    std::array<voices::VoiceMacros, numVoices> macros;

    static constexpr int accentVelocity = 100;   // MIDI velocity >= this => accent

private:
    void renderSegment (juce::AudioBuffer<float>& mainBuffer, Mixer& mixer, float* const* auxChannels, int start, int len);

    std::array<std::unique_ptr<voices::Voice>, numVoices> voiceArray;
    std::vector<float> monoBuf;
    int   maxBlock = 0;
    float accentAmount = 1.5f;
};
}
