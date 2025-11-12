#include "TapeSaturation.h"
#include <cmath>


void TapeSaturation::prepare(const double sampleRate, const int samplesPerBlock, const int numChannels) {

    //lowPassFilter = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 12000.0f);
    //headBumpFilter = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 60.0f, 0.7f, 1.4f);

    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (numChannels);

    lowPassFilters.resize(numChannels);
    headBumpFilters.resize(numChannels);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        lowPassFilters[ch] = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 12000.0f);
        headBumpFilters[ch] = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 60.0f, 0.7f, 1.4f);
        lowPassFilters[ch].prepare(spec);
        headBumpFilters[ch].prepare(spec);
        lowPassFilters[ch].reset();
        headBumpFilters[ch].reset();

    }

    lastHysteresis = 0.0f;
}


void TapeSaturation::processBlock(juce::dsp::AudioBlock<float> block, const float amount, const int numInputChannels)
{

    const float norm = std::sqrt(amount);

    for (int channel = 0; channel < numInputChannels; ++channel) {

        auto channelBlock = block.getSingleChannelBlock(channel);
        juce::dsp::ProcessContextReplacing<float> context(channelBlock);
        lowPassFilters[channel].process(context);
        headBumpFilters[channel].process(context);

        float *channelData = block.getChannelPointer(channel);
        for (int sample = 0; sample < block.getNumSamples(); ++sample) {
            channelData[sample] = hysteresisFilter(channelData[sample]);
            channelData[sample] += bias;
            channelData[sample] = std::tanh(amount * channelData[sample]) / (1.5f * norm);
            channelData[sample] -= bias * 0.5f;
        }
    }
}


float TapeSaturation::hysteresisFilter(const float x)
{
    constexpr float alpha = 0.05f;
    lastHysteresis = alpha * lastHysteresis + (1.0f - alpha) * x;
    return lastHysteresis;
}


