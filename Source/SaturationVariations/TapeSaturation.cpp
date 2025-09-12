#include "TapeSaturation.h"
#include <cmath>


void TapeSaturation::prepare(const double sampleRate, const int samplesPerBlock, const int numChannels) {

    lowPassFilter = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 12000.0f);
    headBumpFilter = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 60.0f, 0.7f, 1.4f);

    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (numChannels);

    lowPassFilter.prepare(spec);
    lowPassFilter.reset();

    headBumpFilter.prepare(spec);
    headBumpFilter.reset();

    lastHysteresis = 0.0f;
}


void TapeSaturation::processBlock(juce::dsp::AudioBlock<float> block, const float amount, const int numInputChannels)
{
    const juce::dsp::ProcessContextReplacing context (block);
    headBumpFilter.process (context);
    lowPassFilter.process (context);

    const float norm = std::sqrt(amount);

    for (int channel = 0; channel < numInputChannels; ++channel) {
        float *channelData = block.getChannelPointer(channel);
        for (int sample = 0; sample < block.getNumSamples(); ++sample) {
            channelData[sample] = hysteresisFilter(channelData[sample]);
            channelData[sample] += bias;
            channelData[sample] = std::tanh(amount * channelData[sample]) / (0.9f * norm);
            channelData[sample] -= bias * 0.5f;
        }
    }
}


float TapeSaturation::hysteresisFilter(const float x)
{
    constexpr float alpha = 0.15f;
    lastHysteresis = alpha * lastHysteresis + (1.0f - alpha) * x;
    return lastHysteresis;
}


