#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class SaturationAudioProcessor;

class SpectrumAnalyserComponent final : public juce::Component, juce::Timer{
public:
    enum {
        fftOrder = 11,
        fftSize = 1 << fftOrder, // 2048
        scopeSize = 512
    };

    explicit SpectrumAnalyserComponent(SaturationAudioProcessor& p);

    void pushNextSampleIntoFFTBuffer(float sample) noexcept;
    void drawNextFrameOfSpectrum();
    void timerCallback() override;
    void paint (juce::Graphics& g) override;
    void setAPVTSReference(juce::AudioProcessorValueTreeState& state) { apvts = &state; }


private:
    SaturationAudioProcessor& audioProcessor;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    float fftFifo[fftSize]{};
    float fftBuffer[2 * fftSize]{};
    int fftFifoIndex = 0;
    bool isFFTBlockReady = false;
    float scopeData[scopeSize]{};
    float displayData[scopeSize]{};

    juce::AudioProcessorValueTreeState* apvts = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyserComponent)
};
