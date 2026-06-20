#pragma once

#include <array>
#include <vector>

#include "VoiceDefs.h"

namespace tr808
{
//==============================================================================
// Descriptor table for every Deep-edit (M3) parameter: which voice, the id
// suffix (combined with the voice prefix via macroId), label, range, default
// and whether the range is skewed toward its low end (handy for freq/time).
//
// This is the layout's source of truth. Each voice separately exposes a
// matching set of float pointers via Voice::deepRefs(); the processor pairs
// them up by id and writes APVTS values into the voice each block. The suffixes
// here must match the voice's deepRefs() (and must never collide with a Macro
// suffix: level/tone/decay/snappy/tune).
//==============================================================================
struct DeepDesc
{
    int   voice;
    const char* suffix;
    const char* label;
    float min, max, def;
    bool  skewLow;
};

inline const std::vector<DeepDesc>& deepParamDescs()
{
    static const std::vector<DeepDesc> d = {
        // Bass Drum
        { BD, "freq",       "Tune",          40.0f,   80.0f,   52.0f,  false },
        { BD, "penvamt",    "Pitch Env Amt", 0.0f,    400.0f,  180.0f, false },
        { BD, "penvtime",   "Pitch Env Time",5.0f,    200.0f,  45.0f,  true  },
        { BD, "bodydecay",  "Body Decay",    50.0f,   1500.0f, 300.0f, true  },
        { BD, "clicklvl",   "Click Level",   0.0f,    1.0f,    0.3f,   false },
        { BD, "clicktone",  "Click Tone",    0.0f,    1.0f,    0.5f,   false },
        { BD, "drive",      "Drive",         1.0f,    10.0f,   1.0f,   true  },

        // Snare
        { SD, "o1freq",     "Osc1 Freq",     100.0f,  300.0f,  180.0f, false },
        { SD, "o2freq",     "Osc2 Freq",     200.0f,  500.0f,  330.0f, false },
        { SD, "oscmix",     "Osc Mix",       0.0f,    1.0f,    0.5f,   false },
        { SD, "shelldecay", "Shell Decay",   20.0f,   400.0f,  130.0f, true  },
        { SD, "nbpfreq",    "Noise BP",      800.0f,  4000.0f, 1800.0f,true  },
        { SD, "nbpq",       "Noise Q",       0.3f,    3.0f,    1.1f,   false },
        { SD, "ndecay",     "Noise Decay",   20.0f,   600.0f,  200.0f, true  },
        { SD, "hpf",        "HPF",           100.0f,  2000.0f, 300.0f, true  },
        { SD, "balance",    "Shell/Noise",   0.0f,    1.0f,    0.5f,   false },

        // Toms
        { LT, "freq",       "Tune",          40.0f,   200.0f,  90.0f,  true  },
        { LT, "penvamt",    "Pitch Env Amt", 0.0f,    1.0f,    0.6f,   false },
        { LT, "penvtime",   "Pitch Env Time",5.0f,    150.0f,  40.0f,  true  },
        { LT, "decaytime",  "Decay",         50.0f,   2000.0f, 600.0f, true  },
        { LT, "atknoise",   "Attack Noise",  0.0f,    1.0f,    0.3f,   false },
        { LT, "drive",      "Drive",         1.0f,    10.0f,   1.0f,   true  },

        { MT, "freq",       "Tune",          50.0f,   260.0f,  130.0f, true  },
        { MT, "penvamt",    "Pitch Env Amt", 0.0f,    1.0f,    0.6f,   false },
        { MT, "penvtime",   "Pitch Env Time",5.0f,    150.0f,  40.0f,  true  },
        { MT, "decaytime",  "Decay",         50.0f,   2000.0f, 500.0f, true  },
        { MT, "atknoise",   "Attack Noise",  0.0f,    1.0f,    0.3f,   false },
        { MT, "drive",      "Drive",         1.0f,    10.0f,   1.0f,   true  },

        { HT, "freq",       "Tune",          60.0f,   320.0f,  180.0f, true  },
        { HT, "penvamt",    "Pitch Env Amt", 0.0f,    1.0f,    0.6f,   false },
        { HT, "penvtime",   "Pitch Env Time",5.0f,    150.0f,  40.0f,  true  },
        { HT, "decaytime",  "Decay",         50.0f,   1500.0f, 450.0f, true  },
        { HT, "atknoise",   "Attack Noise",  0.0f,    1.0f,    0.3f,   false },
        { HT, "drive",      "Drive",         1.0f,    10.0f,   1.0f,   true  },

        // Congas
        { LC, "freq",       "Tune",          120.0f,  400.0f,  220.0f, true  },
        { LC, "penvamt",    "Pitch Env Amt", 0.0f,    1.0f,    0.6f,   false },
        { LC, "penvtime",   "Pitch Env Time",5.0f,    150.0f,  40.0f,  true  },
        { LC, "decaytime",  "Decay",         30.0f,   800.0f,  250.0f, true  },
        { LC, "atknoise",   "Attack Noise",  0.0f,    1.0f,    0.3f,   false },
        { LC, "drive",      "Drive",         1.0f,    10.0f,   1.0f,   true  },

        { MC, "freq",       "Tune",          150.0f,  460.0f,  280.0f, true  },
        { MC, "penvamt",    "Pitch Env Amt", 0.0f,    1.0f,    0.6f,   false },
        { MC, "penvtime",   "Pitch Env Time",5.0f,    150.0f,  40.0f,  true  },
        { MC, "decaytime",  "Decay",         30.0f,   700.0f,  220.0f, true  },
        { MC, "atknoise",   "Attack Noise",  0.0f,    1.0f,    0.3f,   false },
        { MC, "drive",      "Drive",         1.0f,    10.0f,   1.0f,   true  },

        { HC, "freq",       "Tune",          200.0f,  560.0f,  370.0f, true  },
        { HC, "penvamt",    "Pitch Env Amt", 0.0f,    1.0f,    0.6f,   false },
        { HC, "penvtime",   "Pitch Env Time",5.0f,    150.0f,  40.0f,  true  },
        { HC, "decaytime",  "Decay",         30.0f,   600.0f,  200.0f, true  },
        { HC, "atknoise",   "Attack Noise",  0.0f,    1.0f,    0.3f,   false },
        { HC, "drive",      "Drive",         1.0f,    10.0f,   1.0f,   true  },

        // Rim shot / Claves
        { RS, "freq",       "Tune",          1000.0f, 3000.0f, 1700.0f,true  },
        { RS, "decaytime",  "Decay",         10.0f,   200.0f,  60.0f,  true  },
        { CL, "freq",       "Tune",          1500.0f, 4000.0f, 2500.0f,true  },
        { CL, "decaytime",  "Decay",         10.0f,   200.0f,  50.0f,  true  },

        // Hand Clap
        { CP, "bpfreq",     "BP Centre",     500.0f,  2000.0f, 1000.0f,true  },
        { CP, "bpq",        "BP Q",          0.5f,    4.0f,    1.3f,   false },
        { CP, "npulses",    "Pulses",        1.0f,    6.0f,    3.0f,   false },
        { CP, "spacing",    "Pulse Spacing", 3.0f,    30.0f,   10.0f,  false },
        { CP, "taildecay",  "Tail Decay",    20.0f,   400.0f,  120.0f, true  },

        // Cowbell
        { CB, "o1freq",     "Osc1 Freq",     400.0f,  700.0f,  540.0f, false },
        { CB, "o2freq",     "Osc2 Freq",     600.0f,  1000.0f, 800.0f, false },
        { CB, "oscmix",     "Osc Mix",       0.0f,    1.0f,    0.5f,   false },
        { CB, "bpfreq",     "BP Centre",     1000.0f, 4000.0f, 2640.0f,true  },
        { CB, "bpq",        "BP Q",          0.3f,    3.0f,    0.8f,   false },
        { CB, "decaytime",  "Decay",         100.0f,  1000.0f, 400.0f, true  },

        // Metallic group
        { CY, "hpf",        "HPF",           2000.0f, 9000.0f, 5000.0f,true  },
        { CY, "bpfreq",     "Clang Band",    1500.0f, 6000.0f, 3200.0f,true  },
        { CY, "decaytime",  "Decay",         400.0f,  4000.0f, 1500.0f,true  },
        { CY, "balance",    "Band Balance",  0.0f,    1.0f,    0.5f,   false },

        { OH, "hpf",        "HPF",           3000.0f, 12000.0f,7000.0f,true  },
        { OH, "decaytime",  "Decay",         150.0f,  1500.0f, 500.0f, true  },

        { CH, "hpf",        "HPF",           3000.0f, 12000.0f,7000.0f,true  },
        { CH, "decaytime",  "Decay",         20.0f,   150.0f,  50.0f,  true  },

        // Maracas
        { MA, "hpf",        "HPF",           3000.0f, 12000.0f,6000.0f,true  },
        { MA, "decaytime",  "Decay",         10.0f,   100.0f,  30.0f,  true  },
    };
    return d;
}
}
