#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <memory>
#include <vector>

#include "VoiceDefs.h"
#include "Mixer.h"
#include "../voices/Voice.h"
#include "../voices/BassDrumVoice.h"
#include "../dsp/ReverbLexicon.h"
#include "../dsp/StereoDelay.h"

namespace tr808
{
// A single voice trigger at a sample offset within the current block. Built
// from incoming MIDI and/or the Sequencer, merged and rendered sample-accurately.
struct TriggerEvent
{
    int   samplePos;
    int   voiceIndex;       // 0..numVoices-1, or VoiceManager::bassVoiceIndex
    float velocity;
    bool  accent;
    float freqHz = 0.0f;    // melodic bass note frequency (0 = n/a)
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

    // Melodic "808 bass": a dedicated pitched BassDrumVoice, separate from the 16
    // drum voices. Triggered by events whose voiceIndex == bassVoiceIndex.
    static constexpr int bassVoiceIndex = numVoices;
    void  noteOnBass (float velocity, float freqHz, bool accent);
    void  setBassGate (float g) noexcept { bassGate = juce::jlimit (0.0f, 1.0f, g); }
    void  setBassLevel (float v) noexcept { bassLevel = v; }
    void  setBassTone  (float v) noexcept { bassTone = v; }
    void  setBassDecay (float ms) noexcept { bassDecayMs = ms; }
    void  setBassPunch (float v) noexcept { bassPunch = v; }
    void  setBassDrive (float v) noexcept { bassDrive = v; }
    void  setBassDuckBd (bool on) noexcept { bassDuckEnabled = on; }
    bool  isBassActive() const { return bassVoice != nullptr && bassVoice->isActive(); }

    void setAccentAmount (float a) noexcept { accentAmount = juce::jmax (1.0f, a); }

    // Parallel FX sends (0..1). bassVoiceIndex routes the melodic bass.
    void setReverbSend (int v, float a) noexcept;
    void setDelaySend  (int v, float a) noexcept;
    void setReverbReturn (float a) noexcept { reverbReturn = juce::jlimit (0.0f, 1.5f, a); }
    void setDelayReturn  (float a) noexcept { delayReturn  = juce::jlimit (0.0f, 1.5f, a); }
    dsp::ReverbLexicon& reverb() noexcept { return reverbFx; }
    dsp::StereoDelay&   delay()  noexcept { return delayFx; }

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
    std::unique_ptr<voices::BassDrumVoice> bassVoice;   // melodic 808 bass
    voices::VoiceMacros bassMacros;
    float bassGate = 0.5f;                              // note length -> ring (0..1)
    float bassGain = 0.9f;
    float bassLevel = 0.9f, bassTone = 0.25f, bassDecayMs = 650.0f, bassPunch = 2.0f, bassDrive = 1.0f;
    float* bassDeepDecay = nullptr;                     // cached deepRef pointers
    float* bassDeepPunch = nullptr;
    float* bassDeepDrive = nullptr;

    // Ducking: a bass note dips the regular BD (sidechain feel), recovering smoothly.
    bool  bassDuckEnabled = true;
    float bdDuckGain = 1.0f;
    float duckAmount = 0.6f;        // ducked level when a bass note hits
    float duckRelease = 0.0003f;    // per-sample recovery toward 1.0 (set in prepare)
    std::vector<float> monoBuf;
    int   maxBlock = 0;
    float accentAmount = 1.5f;

    // Parallel FX: send buffers (mono) + a reverb and a ping-pong delay.
    dsp::ReverbLexicon reverbFx;
    dsp::StereoDelay   delayFx;
    std::array<float, numVoices + 1> revSend {};        // [numVoices] = bass send
    std::array<float, numVoices + 1> dlySend {};
    std::vector<float> revBuf, dlyBuf, wetL, wetR;
    float reverbReturn = 1.0f, delayReturn = 1.0f;
};
}
