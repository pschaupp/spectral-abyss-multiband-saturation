#pragma once
#include "SaturationProcessor.h"

class TanhSaturation final : public SaturationProcessor {
    public:
    void prepare(double sampleRate, int samplesPerBlock, int numChannels) override{}

    void processBlock(const juce::dsp::AudioBlock<float> block, const float amount, const int numInputChannels) override {
        const float norm = std::sqrt(amount);
        for (int channel = 0; channel < numInputChannels; ++channel) {
            float *channelData = block.getChannelPointer(channel);
            for (int sample = 0; sample < block.getNumSamples(); ++sample) {
                channelData[sample] = std::tanh(amount * channelData[sample]) / (0.9f * norm);
            }
        }
    }
};
