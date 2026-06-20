#pragma once

#include "Voice.h"
#include "../dsp/BandlimitedOsc.h"
#include "../dsp/SVFilter.h"
#include "../dsp/Envelope.h"

namespace tr808::voices
{
// CB: two detuned squares (~540 + ~800 Hz) through a band-pass, with a sharp
// attack and medium decay. Macro: Level.
class CowbellVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

private:
    dsp::BandlimitedOsc o1, o2;
    dsp::SVFilter       bp;
    dsp::Envelope       env;
    float amp = 0.0f;
};
}
