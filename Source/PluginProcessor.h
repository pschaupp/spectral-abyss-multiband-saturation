#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SaturationVariations/SaturationProcessor.h"
#include "SaturationVariations/TanhSaturation.h"
#include "SaturationVariations/TapeSaturation.h"


class SaturationAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    SaturationAudioProcessor();
    ~SaturationAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


    juce::AudioProcessorValueTreeState& getState() { return apvts; }

    void pushNextSampleIntoFifo (float sample);

    juce::AbstractFifo abstractFifo {2048};
    std::vector<float> fifoBuffer = std::vector<float>(2048);



private:
    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // Crossover Filters
    juce::dsp::LinkwitzRileyFilter<float> lowMidLower;
    juce::dsp::LinkwitzRileyFilter<float> lowMidHigher;
    juce::dsp::LinkwitzRileyFilter<float> midHighLower;
    juce::dsp::LinkwitzRileyFilter<float> midHighHigher;

    juce::AudioBuffer<float> lowBuf, midBuf, midTempBuf, highBuf;

    // Saturation
    std::unique_ptr<SaturationProcessor> saturationProcessor;

    std::unique_ptr<TanhSaturation> tanhSaturationLow;
    std::unique_ptr<TanhSaturation> tanhSaturationMid;
    std::unique_ptr<TanhSaturation> tanhSaturationHigh;

    std::unique_ptr<TapeSaturation> tapeSaturationLow;
    std::unique_ptr<TapeSaturation> tapeSaturationMid;
    std::unique_ptr<TapeSaturation> tapeSaturationHigh;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturationAudioProcessor)
};
