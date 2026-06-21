#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

#include "VoiceDefs.h"
#include "../dsp/Saturator.h"

namespace tr808
{
//==============================================================================
// Per-voice pan + mute/solo and the master drive/limiter.
//
// The voice's own Level macro is its fader (applied inside the voice), so the
// mixer only adds pan and the mute/solo gate for the master mix. Per-voice aux
// (multi-out) sends carry the clean mono voice (no pan/gate). Master drive +
// gentle limiting is an oversampled tanh on the stereo master.
//==============================================================================
class Mixer
{
public:
    struct VoiceGain { float gate; float panL; float panR; };

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setPan  (int voice, float pan)  { if (valid (voice)) panValue[(size_t) voice] = juce::jlimit (-1.0f, 1.0f, pan); }
    void setMute (int voice, bool m)     { if (valid (voice)) muted[(size_t) voice]  = m; }
    void setSolo (int voice, bool s)     { if (valid (voice)) soloed[(size_t) voice] = s; }
    void setMasterDrive (float linear)   { masterDrive = juce::jmax (1.0f, linear); }

    bool isMuted (int voice) const { return valid (voice) && muted[(size_t) voice]; }
    bool isSoloed (int voice) const { return valid (voice) && soloed[(size_t) voice]; }
    bool anySolo() const;

    // Gate (mute/solo) + equal-power pan gains for a voice in the master mix.
    VoiceGain gainsFor (int voice) const;

    // Master drive + gentle limit on the stereo bus.
    void processMaster (juce::AudioBuffer<float>& stereo);

private:
    static bool valid (int v) { return v >= 0 && v < numVoices; }

    std::array<float, numVoices> panValue {};   // -1..1, 0 = centre
    std::array<bool,  numVoices> muted  {};
    std::array<bool,  numVoices> soloed {};

    float        masterDrive = 1.0f;
    dsp::Saturator masterSat;
};
}
