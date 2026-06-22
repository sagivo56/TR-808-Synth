#include "PresetManager.h"

#include "VoiceDefs.h"
#include "DeepParams.h"
#include "../params/ParameterIDs.h"

#include <functional>
#include <vector>
#include <utility>

namespace tr808
{
//== Kit param set (synthesis sound) ==========================================
static void forEachKitParam (std::function<void (const juce::String&)> fn)
{
    for (int v = 0; v < numVoices; ++v)
    {
        const auto& s = voiceSpecs()[(size_t) v];
        fn (macroId (v, "level"));
        if (s.tone)   fn (macroId (v, "tone"));
        if (s.decay)  fn (macroId (v, "decay"));
        if (s.snappy) fn (macroId (v, "snappy"));
        if (s.tune)   fn (macroId (v, "tune"));
    }
    for (const auto& d : deepParamDescs())
        fn (macroId (d.voice, d.suffix));
    fn (ParamIDs::masterDrive);
}

static void setParam (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, float real)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (real));
}

struct KitDef { const char* name; std::vector<std::pair<juce::String, float>> overrides; };

static const std::vector<KitDef>& kits()
{
    static const std::vector<KitDef> k = {
        { "Classic 808", {} },
        { "Punchy", { { macroId (BD, "bodydecay"), 180.0f }, { macroId (BD, "clicklvl"), 0.6f },
                      { macroId (BD, "drive"), 2.5f }, { macroId (SD, "ndecay"), 120.0f },
                      { ParamIDs::masterDrive, 1.6f } } },
        { "Deep",   { { macroId (BD, "freq"), 45.0f }, { macroId (BD, "bodydecay"), 800.0f },
                      { macroId (BD, "penvamt"), 120.0f }, { macroId (BD, "penvtime"), 70.0f } } },
        { "Bright", { { macroId (CY, "hpf"), 7000.0f }, { macroId (OH, "hpf"), 9000.0f },
                      { macroId (CH, "hpf"), 9000.0f }, { macroId (SD, "nbpfreq"), 2600.0f },
                      { ParamIDs::masterDrive, 1.3f } } },
    };
    return k;
}

juce::StringArray PresetManager::kitNames()
{
    juce::StringArray names;
    for (const auto& kit : kits()) names.add (kit.name);
    return names;
}

void PresetManager::applyFactoryKit (juce::AudioProcessorValueTreeState& apvts, int index)
{
    if (index < 0 || index >= (int) kits().size())
        return;

    // Reset the kit (synthesis) params to default, then apply the overrides.
    forEachKitParam ([&] (const juce::String& id)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->getDefaultValue());
    });
    for (const auto& ov : kits()[(size_t) index].overrides)
        setParam (apvts, ov.first, ov.second);
}

//== Factory patterns =========================================================
juce::StringArray PresetManager::patternNames()
{
    return { "Four on Floor", "Hip-Hop", "Electro", "Clave Groove" };
}

void PresetManager::applyFactoryPattern (Sequencer& seq, int index)
{
    constexpr int p = 0, var = 0;
    seq.setCurrentPattern (0);
    for (int v = 0; v < numVoices; ++v)
        for (int s = 0; s < Sequencer::maxSteps; ++s)
            seq.setStep (p, var, v, s, false);
    for (int s = 0; s < Sequencer::maxSteps; ++s)
        seq.setAccent (p, var, s, false);
    seq.setLength (p, var, 16);

    auto on  = [&] (int v, int s) { seq.setStep (p, var, v, s, true); };
    auto acc = [&] (int s)        { seq.setAccent (p, var, s, true); };

    switch (index)
    {
        case 1: // Hip-Hop
            on (BD, 0); on (BD, 6); on (BD, 10);
            on (SD, 4); on (SD, 12);
            for (int s = 0; s < 16; ++s) on (CH, s);
            on (OH, 2); on (OH, 14); acc (0); acc (4);
            break;
        case 2: // Electro
            for (int s : { 0, 3, 6, 10, 12 }) on (BD, s);
            on (SD, 4); on (SD, 12);
            for (int s = 2; s < 16; s += 4) on (CH, s);
            on (CP, 4); on (CP, 12); acc (0);
            break;
        case 3: // Clave Groove
            on (BD, 0); on (BD, 8);
            for (int s : { 0, 3, 6, 10, 12 }) on (CL, s);
            on (MA, 2); on (MA, 6); on (MA, 10); on (MA, 14);
            on (LC, 4); on (HC, 12); acc (0);
            break;
        default: // Four on Floor
            for (int s : { 0, 4, 8, 12 }) on (BD, s);
            on (SD, 4); on (SD, 12);
            for (int s = 0; s < 16; s += 2) on (CH, s);
            acc (0); acc (8);
            break;
    }
}

//== Musical random ===========================================================
void PresetManager::randomizeKit (juce::AudioProcessorValueTreeState& apvts)
{
    juce::Random r;
    forEachKitParam ([&] (const juce::String& id)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (0.2f + 0.6f * r.nextFloat());   // mid-biased -> stays musical
    });
}

void PresetManager::randomizePattern (Sequencer& seq)
{
    juce::Random r;
    constexpr int p = 0, var = 0, len = 16;
    seq.setCurrentPattern (0);
    for (int v = 0; v < numVoices; ++v)
        for (int s = 0; s < Sequencer::maxSteps; ++s)
            seq.setStep (p, var, v, s, false);
    for (int s = 0; s < Sequencer::maxSteps; ++s) seq.setAccent (p, var, s, false);
    seq.setLength (p, var, len);

    auto chance = [&] (float prob) { return r.nextFloat() < prob; };

    // Kick: downbeat + likely beat-3, light syncopation.
    seq.setStep (p, var, BD, 0, true);
    if (chance (0.6f)) seq.setStep (p, var, BD, 8, true);
    for (int s : { 3, 6, 10, 11, 14 }) if (chance (0.18f)) seq.setStep (p, var, BD, s, true);

    // Snare: backbeat, occasional ghost.
    seq.setStep (p, var, SD, 4, true);
    seq.setStep (p, var, SD, 12, true);
    if (chance (0.15f)) seq.setStep (p, var, SD, 14, true);

    // Hats: steady 8ths or 16ths.
    const bool sixteenths = chance (0.4f);
    for (int s = 0; s < len; ++s)
        if ((sixteenths || s % 2 == 0) && chance (0.85f))
            seq.setStep (p, var, chance (0.15f) ? OH : CH, s, true);

    // Sparse percussion.
    if (chance (0.4f)) { seq.setStep (p, var, CP, 4, true); seq.setStep (p, var, CP, 12, true); }
    for (int s : { 2, 6, 10, 14 }) if (chance (0.15f)) seq.setStep (p, var, chance (0.5f) ? MA : CB, s, true);
    if (chance (0.3f)) seq.setStep (p, var, LT, r.nextInt (len), true);

    seq.setAccent (p, var, 0, true);
    if (chance (0.5f)) seq.setAccent (p, var, 8, true);
}

//== User save / load =========================================================
juce::File PresetManager::presetsDir()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("TR-808 Synth").getChildFile ("Presets");
    dir.createDirectory();
    return dir;
}

void PresetManager::saveKit (juce::AudioProcessorValueTreeState& apvts, const juce::File& file)
{
    if (auto xml = apvts.copyState().createXml())
        file.replaceWithText (xml->toString());
}

bool PresetManager::loadKit (juce::AudioProcessorValueTreeState& apvts, const juce::File& file)
{
    if (auto xml = juce::XmlDocument::parse (file))
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        if (tree.hasType (apvts.state.getType()))
        {
            apvts.replaceState (tree);
            return true;
        }
    }
    return false;
}

void PresetManager::savePattern (const Sequencer& seq, const juce::File& file)
{
    if (auto xml = seq.toValueTree().createXml())
        file.replaceWithText (xml->toString());
}

bool PresetManager::loadPattern (Sequencer& seq, const juce::File& file)
{
    if (auto xml = juce::XmlDocument::parse (file))
    {
        seq.fromValueTree (juce::ValueTree::fromXml (*xml));
        return true;
    }
    return false;
}
}
