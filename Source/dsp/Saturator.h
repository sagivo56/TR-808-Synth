#pragma once

#include <juce_dsp/juce_dsp.h>
#include <memory>

namespace tr808::dsp
{
//==============================================================================
// tanh soft-clip saturator with a drive control.
//
//  - processSample(): the raw waveshaper, for use where oversampling is handled
//    elsewhere (or not needed).
//  - processBlock():  oversampled (2x/4x) waveshaping to keep the nonlinearity
//    from aliasing. The Oversampling object is allocated in prepare(), never on
//    the audio thread.
//==============================================================================
class Saturator
{
public:
    void prepare (double sampleRate, int maxBlockSize, int numChannels, int oversampleLog2 = 1);
    void reset();

    void setDrive (float driveLinear) noexcept { drive = juce::jmax (1.0f, driveLinear); }

    static float shape (float x, float drive) noexcept;
    float processSample (float x) const noexcept { return shape (x, drive); }

    void processBlock (juce::dsp::AudioBlock<float> block);

    int getLatencySamples() const noexcept;

private:
    float drive = 1.0f;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
};
}
