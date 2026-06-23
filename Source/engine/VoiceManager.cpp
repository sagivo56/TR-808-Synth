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

    std::unique_ptr<Voice> makeResonator (float freq, float decaySec, bool swing)
    {
        auto v = std::make_unique<TunedResonatorVoice>();
        v->setConfig (freq, decaySec);
        v->setSwing (swing);
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
    voiceArray[RS] = makeResonator (1700.0f, 0.06f, true);    // rim shot uses the swing VCA
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
    voiceArray[CL] = makeResonator (2500.0f, 0.05f, false);

    bassVoice = std::make_unique<BassDrumVoice>();   // melodic 808 bass
}

void VoiceManager::prepare (double sampleRate, int maxBlockSize)
{
    maxBlock = juce::jmax (1, maxBlockSize);
    monoBuf.assign ((size_t) maxBlock, 0.0f);

    for (auto& v : voiceArray)
        v->prepare (sampleRate, maxBlock);
    if (bassVoice) bassVoice->prepare (sampleRate, maxBlock);

    if (bassVoice)
        for (auto& r : bassVoice->deepRefs())
        {
            if (r.suffix == "bodydecay") bassDeepDecay = r.ptr;
            else if (r.suffix == "punch") bassDeepPunch = r.ptr;
            else if (r.suffix == "drive") bassDeepDrive = r.ptr;
        }

    duckRelease = (float) (1.0 / (0.06 * sampleRate));   // ~60 ms recovery
    bdDuckGain = 1.0f;

    reverbFx.prepare (sampleRate);
    delayFx.prepare (sampleRate);
    revBuf.assign ((size_t) maxBlock, 0.0f);
    dlyBuf.assign ((size_t) maxBlock, 0.0f);
    wetL.assign  ((size_t) maxBlock, 0.0f);
    wetR.assign  ((size_t) maxBlock, 0.0f);
}

void VoiceManager::setReverbSend (int v, float a) noexcept
{
    if (v >= 0 && v <= numVoices) revSend[(size_t) v] = juce::jlimit (0.0f, 1.0f, a);
}

void VoiceManager::setDelaySend (int v, float a) noexcept
{
    if (v >= 0 && v <= numVoices) dlySend[(size_t) v] = juce::jlimit (0.0f, 1.0f, a);
}

void VoiceManager::reset()
{
    for (auto& v : voiceArray)
        v->reset();
    if (bassVoice) bassVoice->reset();
    reverbFx.reset();
    delayFx.reset();
}

void VoiceManager::noteOnBass (float velocity, float freqHz, bool accent)
{
    if (bassVoice == nullptr || freqHz <= 0.0f)
        return;
    bassMacros.level = bassLevel;
    bassMacros.tone  = bassTone;              // click / attack
    bassMacros.decay = juce::jlimit (0.0f, 1.0f, bassGate);   // per-note length -> ring
    bassMacros.tune  = 0.5f;
    if (bassDeepDecay) *bassDeepDecay = bassDecayMs;          // base ring
    if (bassDeepPunch) *bassDeepPunch = bassPunch;
    if (bassDeepDrive) *bassDeepDrive = bassDrive;
    if (bassDuckEnabled) bdDuckGain = duckAmount;            // dip the regular BD
    bassVoice->setMacros (bassMacros);
    bassVoice->setPlayFrequency (freqHz);
    bassVoice->trigger (accent ? velocity * accentAmount : velocity, accent);
}

void VoiceManager::noteOn (int voiceIndex, float velocity, bool accent)
{
    if (voiceIndex < 0 || voiceIndex >= numVoices)
        return;

    // Hi-hat choke group: open and closed hat are one source, mutually exclusive.
    if (voiceIndex == CH) voiceArray[OH]->choke();
    if (voiceIndex == OH) voiceArray[CH]->choke();

    voiceArray[(size_t) voiceIndex]->setMacros (macros[(size_t) voiceIndex]);
    voiceArray[(size_t) voiceIndex]->trigger (accent ? velocity * accentAmount : velocity, accent);
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

    const float duckStart = bdDuckGain;   // BD ducking ramp for this segment

    for (int v = 0; v < numVoices; ++v)
    {
        if (! voiceArray[(size_t) v]->isActive())
            continue;

        std::fill (scratch, scratch + len, 0.0f);
        voiceArray[(size_t) v]->renderAdd (scratch, len);

        // BD ducking: dip the regular BD while a bass note rings, recovering.
        if (v == BD && bassDuckEnabled)
        {
            float dg = duckStart;
            for (int i = 0; i < len; ++i) { scratch[i] *= dg; dg += (1.0f - dg) * duckRelease; }
        }

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

        // Parallel FX sends (pre-pan mono).
        const float rs = revSend[(size_t) v], ds = dlySend[(size_t) v];
        if (rs > 0.0f) { auto* b = revBuf.data() + start; for (int i = 0; i < len; ++i) b[i] += scratch[i] * rs; }
        if (ds > 0.0f) { auto* b = dlyBuf.data() + start; for (int i = 0; i < len; ++i) b[i] += scratch[i] * ds; }
    }

    // Melodic 808 bass: centre-panned, master only (+ its own FX sends).
    if (bassVoice != nullptr && bassVoice->isActive())
    {
        std::fill (scratch, scratch + len, 0.0f);
        bassVoice->renderAdd (scratch, len);
        const float brs = revSend[(size_t) numVoices], bds = dlySend[(size_t) numVoices];
        for (int i = 0; i < len; ++i)
        {
            const float s = scratch[i] * bassGain;
            L[start + i] += s;
            R[start + i] += s;
        }
        if (brs > 0.0f) { auto* b = revBuf.data() + start; for (int i = 0; i < len; ++i) b[i] += scratch[i] * brs; }
        if (bds > 0.0f) { auto* b = dlyBuf.data() + start; for (int i = 0; i < len; ++i) b[i] += scratch[i] * bds; }
    }

    // Advance the BD-duck recovery for this segment (even if BD was silent).
    for (int i = 0; i < len; ++i) bdDuckGain += (1.0f - bdDuckGain) * duckRelease;
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

    const int ns = juce::jmin (numSamples, maxBlock);
    std::fill (revBuf.begin(), revBuf.begin() + ns, 0.0f);
    std::fill (dlyBuf.begin(), dlyBuf.begin() + ns, 0.0f);

    int pos = 0;
    for (const auto& e : events)
    {
        const int t = juce::jlimit (0, numSamples, e.samplePos);
        renderSegment (mainBuffer, mixer, auxChannels, pos, t - pos);
        pos = t;
        if (e.voiceIndex == bassVoiceIndex) noteOnBass (e.velocity, e.freqHz, e.accent);
        else                                noteOn (e.voiceIndex, e.velocity, e.accent);
    }

    renderSegment (mainBuffer, mixer, auxChannels, pos, numSamples - pos);

    // Parallel FX returns: process the send buses (always, so tails ring out)
    // and add the stereo wet to the master.
    auto* L = mainBuffer.getWritePointer (0);
    auto* R = mainBuffer.getNumChannels() > 1 ? mainBuffer.getWritePointer (1) : L;

    reverbFx.process (revBuf.data(), wetL.data(), wetR.data(), ns);
    for (int i = 0; i < ns; ++i) { L[i] += wetL[(size_t) i] * reverbReturn; R[i] += wetR[(size_t) i] * reverbReturn; }

    delayFx.process (dlyBuf.data(), wetL.data(), wetR.data(), ns);
    for (int i = 0; i < ns; ++i) { L[i] += wetL[(size_t) i] * delayReturn; R[i] += wetR[(size_t) i] * delayReturn; }
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
