#pragma once

#include "Voice.h"
#include "../dsp/MetalCluster.h"
#include "../dsp/SVFilter.h"
#include "../dsp/Envelope.h"

namespace tr808::voices
{
// CY / OH / CH: the shared inharmonic MetalCluster shaped per voice.
//  - Cymbal: high band + a band-passed "clang", balanced by Tone, long decay.
//  - Open hat: high-passed cluster, medium-long decay.
//  - Closed hat: high-passed cluster, very short decay.
// The hats live in a choke group (see VoiceManager) and choke() fades fast.
class MetalVoice : public Voice
{
public:
    enum class Type { cymbal, openHat, closedHat };

    void setType (Type t) noexcept { type = t; }

    void prepare (double sr, int maxBlock) override;
    void reset() override;
    void trigger (float velocity, bool accent) override;
    void renderAdd (float* mono, int numSamples) override;
    bool isActive() const override;
    void choke() override;

private:
    dsp::MetalCluster cluster;
    dsp::SVFilter     hpf;
    dsp::SVFilter     bp;        // cymbal "clang" band
    dsp::Envelope     env;

    Type  type    = Type::closedHat;
    float amp     = 0.0f;
    float toneBal = 0.5f;
};
}
