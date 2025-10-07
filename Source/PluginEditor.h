#pragma once

#include "PluginProcessor.h"
#include "SpectrumAnalyzer/SpectrumAnalyserComponent.h"
#include "CustomLookAndFeel.h"

class SaturationAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit SaturationAudioProcessorEditor (SaturationAudioProcessor&);
    ~SaturationAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


private:
    SaturationAudioProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel;

    SpectrumAnalyserComponent spectrumAnalyser;

    juce::Slider saturationSliderLow;
    juce::Slider saturationSliderMid;
    juce::Slider saturationSliderHigh;
    juce::Slider crossoverSliderLowMid;
    juce::Slider crossoverSliderMidHigh;

    juce::GlowEffect glow;

    juce::Label saturationLabelLow;
    juce::Label saturationLabelMid;
    juce::Label saturationLabelHigh;
    juce::Label crossoverLabelLow;
    juce::Label crossoverLabelHigh;

    juce::ComboBox saturationTypeMenu;

    juce::AudioProcessorValueTreeState::SliderAttachment amountLowAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment amountMidAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment amountHighAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment crossoverLowMidAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment crossoverMidHighAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> saturationTypeMenuAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturationAudioProcessorEditor)
};
