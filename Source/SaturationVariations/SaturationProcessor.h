#pragma once
#include <cmath>

class SaturationProcessor {
public:
    virtual ~SaturationProcessor() = default;
    virtual void prepare(double sampleRate, int samplesPerBlock, int numChannels) = 0;
    virtual void processBlock(juce::dsp::AudioBlock<float> block, float amount, int numInputChannels) = 0;
};

