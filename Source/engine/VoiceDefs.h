#pragma once

#include <array>
#include <string>

namespace tr808
{
//==============================================================================
// Canonical voice order, GM note mapping and which Macro knobs each voice has.
// Shared by the parameter layout, the processor (APVTS wiring) and the
// VoiceManager so they never drift out of sync.
//==============================================================================
enum VoiceIndex
{
    BD = 0, RS, SD, CP, LT, MT, HT, MA, CB, CY, OH, CH, LC, MC, HC, CL,
    numVoices
};

struct VoiceSpec
{
    const char* prefix;   // parameter-id prefix, e.g. "bd"
    const char* name;     // human label
    int  gmNote;          // default General-MIDI trigger note
    bool tone, decay, snappy, tune;   // which Macro params this voice exposes (Level is universal)
};

inline const std::array<VoiceSpec, numVoices>& voiceSpecs()
{
    static const std::array<VoiceSpec, numVoices> specs = { {
        { "bd", "Bass Drum",  36, true,  true,  false, true  },
        { "rs", "Rim Shot",   37, false, false, false, false },
        { "sd", "Snare",      38, true,  false, true,  false },
        { "cp", "Hand Clap",  39, false, false, false, false },
        { "lt", "Low Tom",    41, false, false, false, true  },
        { "mt", "Mid Tom",    47, false, false, false, true  },
        { "ht", "Hi Tom",     50, false, false, false, true  },
        { "ma", "Maracas",    70, false, false, false, false },
        { "cb", "Cowbell",    56, false, false, false, false },
        { "cy", "Cymbal",     49, true,  true,  false, false },
        { "oh", "Open Hat",   46, false, true,  false, false },
        { "ch", "Closed Hat", 42, false, false, false, false },
        { "lc", "Low Conga",  64, false, false, false, true  },
        { "mc", "Mid Conga",  63, false, false, false, true  },
        { "hc", "Hi Conga",   62, false, false, false, true  },
        { "cl", "Claves",     75, false, false, false, false },
    } };
    return specs;
}

inline int gmNoteToVoice (int note) noexcept
{
    for (int i = 0; i < numVoices; ++i)
        if (voiceSpecs()[(size_t) i].gmNote == note)
            return i;
    return -1;
}

inline std::string macroId (int idx, const char* macro)
{
    return std::string (voiceSpecs()[(size_t) idx].prefix) + "_" + macro;
}
}
