#include "VoiceManager.h"

#include "../voices/BassDrumVoice.h"
#include "../voices/SnareVoice.h"
#include "../voices/TomCongaVoice.h"
#include "../voices/TunedResonatorVoice.h"
#include "../voices/ClapVoice.h"
#include "../voices/CowbellVoice.h"
#include "../voices/MetalVoice.h"
#include "../voices/MaracasVoice.h"

namespace tr808
{
using namespace voices;

namespace
{
    std::unique_ptr<Voice> makeTomConga (float freq, float decaySec)
    {
        auto v = std::make_unique<TomCongaVoice>();
        v->setConfig (freq, decaySec);
        return v;
    }

    std::unique_ptr<Voice> makeResonator (float freq, float decaySec)
    {
        auto v = std::make_unique<TunedResonatorVoice>();
        v->setConfig (freq, decaySec);
        return v;
    }

    std::unique_ptr<Voice> makeMetal (MetalVoice::Type type)
    {
        auto v = std::make_unique<MetalVoice>();
        v->setType (type);
        return v;
    }
}

VoiceManager::VoiceManager()
{
    voiceArray[BD] = std::make_unique<BassDrumVoice>();
    voiceArray[RS] = makeResonator (1700.0f, 0.06f);
    voiceArray[SD] = std::make_unique<SnareVoice>();
    voiceArray[CP] = std::make_unique<ClapVoice>();
    voiceArray[LT] = makeTomConga (90.0f,  0.60f);
    voiceArray[MT] = makeTomConga (130.0f, 0.50f);
    voiceArray[HT] = makeTomConga (180.0f, 0.45f);
    voiceArray[MA] = std::make_unique<MaracasVoice>();
    voiceArray[CB] = std::make_unique<CowbellVoice>();
    voiceArray[CY] = makeMetal (MetalVoice::Type::cymbal);
    voiceArray[OH] = makeMetal (MetalVoice::Type::openHat);
    voiceArray[CH] = makeMetal (MetalVoice::Type::closedHat);
    voiceArray[LC] = makeTomConga (220.0f, 0.25f);
    voiceArray[MC] = makeTomConga (280.0f, 0.22f);
    voiceArray[HC] = makeTomConga (370.0f, 0.20f);
    voiceArray[CL] = makeResonator (2500.0f, 0.05f);
}

void VoiceManager::prepare (double sampleRate, int maxBlockSize)
{
    maxBlock = juce::jmax (1, maxBlockSize);
    monoBuf.assign ((size_t) maxBlock, 0.0f);

    for (auto& v : voiceArray)
        v->prepare (sampleRate, maxBlock);
}

void VoiceManager::reset()
{
    for (auto& v : voiceArray)
        v->reset();
}

void VoiceManager::noteOn (int voiceIndex, float velocity, bool accent)
{
    if (voiceIndex < 0 || voiceIndex >= numVoices)
        return;

    // Hi-hat choke group: open and closed hat are one source, mutually exclusive.
    if (voiceIndex == CH) voiceArray[OH]->choke();
    if (voiceIndex == OH) voiceArray[CH]->choke();

    voiceArray[(size_t) voiceIndex]->setMacros (macros[(size_t) voiceIndex]);
    voiceArray[(size_t) voiceIndex]->trigger (velocity, accent);
}

bool VoiceManager::isVoiceActive (int voiceIndex) const
{
    if (voiceIndex < 0 || voiceIndex >= numVoices)
        return false;
    return voiceArray[(size_t) voiceIndex]->isActive();
}

void VoiceManager::renderSegment (juce::AudioBuffer<float>& mainBuffer, Mixer& mixer, float* const* auxChannels, int start, int len)
{
    if (len <= 0)
        return;

    len = juce::jmin (len, maxBlock);
    auto* L = mainBuffer.getWritePointer (0);
    auto* R = mainBuffer.getNumChannels() > 1 ? mainBuffer.getWritePointer (1) : L;
    auto* scratch = monoBuf.data();

    for (int v = 0; v < numVoices; ++v)
    {
        if (! voiceArray[(size_t) v]->isActive())
            continue;

        std::fill (scratch, scratch + len, 0.0f);
        voiceArray[(size_t) v]->renderAdd (scratch, len);

        // Master: panned + mute/solo gated.
        const auto g = mixer.gainsFor (v);
        if (g.gate > 0.0f)
        {
            const float gl = g.gate * g.panL;
            const float gr = g.gate * g.panR;
            for (int i = 0; i < len; ++i)
            {
                const float s = scratch[i];
                L[start + i] += s * gl;
                R[start + i] += s * gr;
            }
        }

        // Aux send (multi-out): the clean mono voice, no pan/gate.
        if (auxChannels != nullptr && auxChannels[v] != nullptr)
        {
            auto* a = auxChannels[v] + start;
            for (int i = 0; i < len; ++i)
                a[i] += scratch[i];
        }
    }
}

void VoiceManager::renderEvents (juce::AudioBuffer<float>& mainBuffer,
                                 const std::vector<TriggerEvent>& events,
                                 Mixer& mixer,
                                 float* const* auxChannels)
{
    const int numSamples = mainBuffer.getNumSamples();
    mainBuffer.clear();
    if (auxChannels != nullptr)
        for (int v = 0; v < numVoices; ++v)
            if (auxChannels[v] != nullptr)
                juce::FloatVectorOperations::clear (auxChannels[v], numSamples);

    int pos = 0;
    for (const auto& e : events)
    {
        const int t = juce::jlimit (0, numSamples, e.samplePos);
        renderSegment (mainBuffer, mixer, auxChannels, pos, t - pos);
        pos = t;
        noteOn (e.voiceIndex, e.velocity, e.accent);
    }

    renderSegment (mainBuffer, mixer, auxChannels, pos, numSamples - pos);
}

void VoiceManager::process (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    // Test convenience: translate MIDI note-ons to events and render through a
    // default (centre-pan, nothing muted) mixer with no aux sends.
    std::vector<TriggerEvent> events;
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
        {
            const int idx = gmNoteToVoice (msg.getNoteNumber());
            if (idx >= 0)
                events.push_back ({ meta.samplePosition, idx, msg.getFloatVelocity(),
                                    msg.getVelocity() >= accentVelocity });
        }
    }
    Mixer defaultMixer;
    std::array<float*, numVoices> noAux {};
    renderEvents (buffer, events, defaultMixer, noAux.data());
}
}
