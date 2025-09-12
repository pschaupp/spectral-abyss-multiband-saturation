#pragma once
#include <juce_dsp/juce_dsp.h>

#include "SaturationProcessor.h"

class TapeSaturation final : public SaturationProcessor {
public:
    void prepare(double sampleRate, int samplesPerBlock, int numChannels) override;
    void processBlock(juce::dsp::AudioBlock<float> block, float amount, int numInputChannels) override;

private:
    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    int numChannels = 1;

    // Filters
    juce::dsp::IIR::Filter<float> lowPassFilter;
    juce::dsp::IIR::Filter<float> headBumpFilter;

    // Parameters
    float amount = 1.0f;
    float bias = 0.0f;

    // Hysteresis
    float lastHysteresis = 0.0f;
    float hysteresisFilter(float x);
};


