// Offline tone dump: render a single voice to a raw float32 mono file so the
// Python analysis scripts can measure it against the real-808 reference.
//
//   tone_dump <bd|ch|oh|cy> <seconds> <out.f32> [deepSuffix=value ...]
//
// Deep params are set through the voice's own deepRefs() pointers, so e.g.
//   tone_dump ch 0.4 ch.f32 hpf=3500 decaytime=60
// sweeps the closed-hat HPF cutoff and decay with no recompile.

#include <juce_audio_basics/juce_audio_basics.h>

#include "voices/BassDrumVoice.h"
#include "voices/MetalVoice.h"

#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <iostream>

using namespace tr808::voices;

int main (int argc, char** argv)
{
    if (argc < 4) { std::cerr << "usage: tone_dump <bd|ch|oh|cy> <seconds> <out.f32> [suffix=val ...]\n"; return 2; }

    const std::string which = argv[1];
    const double seconds = std::atof (argv[2]);
    const std::string out = argv[3];
    constexpr double sr = 48000.0;

    std::unique_ptr<Voice> v;
    if (which == "bd") v = std::make_unique<BassDrumVoice>();
    else
    {
        auto m = std::make_unique<MetalVoice>();
        m->setType (which == "oh" ? MetalVoice::Type::openHat
                  : which == "cy" ? MetalVoice::Type::cymbal
                                   : MetalVoice::Type::closedHat);
        v = std::move (m);
    }

    v->prepare (sr, 512);

    // apply deep overrides from argv (suffix=value)
    auto refs = v->deepRefs();
    for (int i = 4; i < argc; ++i)
    {
        std::string s = argv[i];
        auto eq = s.find ('=');
        if (eq == std::string::npos) continue;
        const std::string key = s.substr (0, eq);
        const float val = (float) std::atof (s.substr (eq + 1).c_str());
        for (auto& r : refs) if (r.suffix == key) *r.ptr = val;
    }

    v->reset();
    v->trigger (1.0f, false);

    const int total = (int) std::ceil (seconds * sr);
    std::vector<float> blk (512), all;
    all.reserve ((size_t) total);
    int done = 0;
    while (done < total)
    {
        std::fill (blk.begin(), blk.end(), 0.0f);
        v->renderAdd (blk.data(), 512);
        for (int i = 0; i < 512 && done < total; ++i, ++done) all.push_back (blk[(size_t) i]);
    }

    std::ofstream f (out, std::ios::binary);
    f.write (reinterpret_cast<const char*> (all.data()), (std::streamsize) (all.size() * sizeof (float)));
    std::cerr << "wrote " << all.size() << " samples to " << out << "\n";
    return 0;
}
