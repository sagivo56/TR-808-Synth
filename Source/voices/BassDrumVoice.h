#pragma once

#include "Voice.h"
#include "../dsp/ResonatorBT.h"

namespace tr808::voices
{
// BD — service-manual block diagram (OSC IC12 / DECAY / TONE / MONO 841,
// MULTI 842 / LEVEL / BUFFER Q44):
//  * OSC IC12  -> bridged-T resonator (ResonatorBT): a decaying sine whose decay
//    is set by the feedback Q. On trigger it rings at a "punch" multiple of the
//    inherent frequency for the first half-cycle, then drops to the inherent
//    frequency and a retrigger pulse re-excites it (the Q41-Q43 trick).
//  * DECAY     -> ring time (Q/feedback), scaled by the Decay macro + Sustain.
//  * TONE      -> the attack "beater" click fed forward to the output mix (the
//    schematic's TONE pot sits between the trigger pulse and the buffer), so the
//    Tone macro sets how clicky vs. round the attack is (0 = pure sine).
//  * LEVEL     -> output level (Level macro). BUFFER Q44 -> unity output buffer.
class BassDrumVoice : public Voice
{
public:
    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;

    std::vector<DeepRef> deepRefs() override
    {
        return { { "freq", &deep.freq }, { "bodydecay", &deep.decay }, { "punch", &deep.punch },
                 { "retrig", &deep.retrig }, { "sustain", &deep.sustain }, { "drive", &deep.drive } };
    }

private:
    struct Deep
    {
        float freq = 55.0f, decay = 320.0f, punch = 2.0f, retrig = 0.5f, sustain = 0.0f, drive = 1.0f;
    } deep;

    dsp::ResonatorBT res;

    float amp        = 0.0f;
    float baseFreq   = 55.0f;
    float driveAmt   = 1.0f;
    float retrigAmt  = 0.5f;
    int   switchSample = 0;
    int   sampleCount  = 0;
    bool  switched     = true;

    // TONE: feed-forward attack "beater" click (a short, bright, fast-decaying
    // burst whose level is the Tone macro).
    float clickPhase = 0.0f;
    float clickInc   = 0.0f;
    float clickEnv   = 0.0f;
    float clickAmt   = 0.0f;
    float clickDecay = 0.0f;
};
}
