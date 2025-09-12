#include "SpectrumAnalyserComponent.h"
#include "../PluginProcessor.h"

SpectrumAnalyserComponent::SpectrumAnalyserComponent(SaturationAudioProcessor &p)
    : audioProcessor(p),
      forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann) {
    std::copy(scopeData, scopeData + scopeSize, displayData);
    startTimerHz(40);
}

void SpectrumAnalyserComponent::pushNextSampleIntoFFTBuffer(const float sample) noexcept {
    if (fftFifoIndex == fftSize) {
        if (!isFFTBlockReady) {
            juce::zeromem(fftBuffer, sizeof (fftBuffer));
            memcpy(fftBuffer, fftFifo, sizeof (fftFifo));
            isFFTBlockReady = true;
        }
        fftFifoIndex = 0;
    }
    fftFifo[fftFifoIndex++] = sample;
}

void SpectrumAnalyserComponent::drawNextFrameOfSpectrum() {
    window.multiplyWithWindowingTable(fftBuffer, fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftBuffer);

    auto mindB = -70.0f;
    auto maxdB = 0.0f;

    const float sampleRate = audioProcessor.getSampleRate();
    const float nyquist = sampleRate / 2.0f;

    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;

    for (int i = 0; i < scopeSize; ++i) {
        const float proportionX = static_cast<float>(i) / static_cast<float>(scopeSize);

        const float freq = minFreq * std::pow(maxFreq / minFreq, proportionX);
        const int fftDataIndex = static_cast<int>(freq / nyquist * (fftSize / 2));

        // Magnitude to dB → normalised [0..1]
        const float level = juce::jmap(
            juce::jlimit(
                mindB,
                maxdB,
                juce::Decibels::gainToDecibels(fftBuffer[fftDataIndex]) - juce::Decibels::gainToDecibels(static_cast<float>(fftSize))),
            mindB,
            maxdB,
            0.0f,
            1.0f);

        scopeData[i] = level;
    }

    // Smoothing
    const float alpha = 0.6f;
    for (int i = 0; i < scopeSize; ++i) {
        int count = 0;
        float sum = 0.0f;
        for (int j = -6; j <= 6; ++j) {
            if (const int idx = i + j; idx >= 0 && idx < scopeSize) {
                sum += scopeData[idx];
                count++;
            }
        }
        scopeData[i] = sum / static_cast<float>(count);
        displayData[i] = alpha * scopeData[i] + (1.0f - alpha) * displayData[i];
    }
}


void SpectrumAnalyserComponent::timerCallback() {
    int start1, size1, start2, size2;
    audioProcessor.abstractFifo.prepareToRead(512, start1, size1, start2, size2);

    if (size1 > 0) {
        for (int i = 0; i < size1; ++i)
            pushNextSampleIntoFFTBuffer(audioProcessor.fifoBuffer[start1 + i]);
    }

    audioProcessor.abstractFifo.finishedRead(size1);

    if (isFFTBlockReady) {
        drawNextFrameOfSpectrum();
        isFFTBlockReady = false;
        repaint();
    }
}

void SpectrumAnalyserComponent::paint(juce::Graphics &g) {
    juce::Rectangle<int> bounds = getLocalBounds();
    const int width = bounds.getWidth();

    const juce::Rectangle<int> labelArea = bounds.removeFromBottom(20);
    const juce::Rectangle<int> plotArea = bounds;


    // Frequency grid lines
    g.setColour(juce::Colours::darkgrey);
    const std::vector gridFrequencies{20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f};

    for (const float freq: gridFrequencies) {
        g.setColour(juce::Colour(0x20D3D0F2));
        if (freq > audioProcessor.getSampleRate() / 2.0f)
            continue;

        const float proportionX = std::log(freq / 20.0f) / std::log(20000.0f / 20.0f);
        const int x = juce::roundToInt(proportionX * width);

        g.drawVerticalLine(x, static_cast<float>(plotArea.getY()), static_cast<float>(plotArea.getBottom()));

        juce::String label = juce::String(static_cast<int>(freq)) + "Hz";

        g.setColour(juce::Colour(juce::Colours::white));
        g.setFont(10.0f);
        g.drawFittedText(label,
                         x, labelArea.getY(), 50, labelArea.getHeight(),
                         juce::Justification::left, 1);
    }

    // Draw spectrum curve
    g.setColour(juce::Colours::aliceblue);

    for (int i = 1; i < scopeSize; ++i) {
        g.drawLine({
            static_cast<float>(juce::jmap(i - 1, 0, scopeSize - 1, 0, width)),
            juce::jmap(displayData[i - 1], 0.0f, 1.0f, static_cast<float>(plotArea.getBottom()),
                       static_cast<float>(plotArea.getY())),
            static_cast<float>(juce::jmap(i, 0, scopeSize - 1, 0, width)),
            juce::jmap(displayData[i], 0.0f, 1.0f, static_cast<float>(plotArea.getBottom()),
                       static_cast<float>(plotArea.getY()))
        });
    }


    // Draw frequency crossover lines
    g.setColour(juce::Colour(0x80D3D0F2));

    const float lowCrossoverFrequency = apvts->getRawParameterValue("crossoverLowMid")->load();
    const int positionLowCrossover = juce::roundToInt(
        std::log(lowCrossoverFrequency / 20.0f) / std::log(20000.0f / 20.0f) * width);
    g.drawLine(static_cast<float>(positionLowCrossover),
               static_cast<float>(plotArea.getY()),
               static_cast<float>(positionLowCrossover),
               static_cast<float>(plotArea.getBottom()),
               2.0f);

    const float highCrossoverFrequency = apvts->getRawParameterValue("crossoverMidHigh")->load();
    const int positionHighCrossover = juce::roundToInt(
        std::log(highCrossoverFrequency / 20.0f) / std::log(20000.0f / 20.0f) * width);
    g.drawLine(static_cast<float>(positionHighCrossover),
               static_cast<float>(plotArea.getY()),
               static_cast<float>(positionHighCrossover),
               static_cast<float>(plotArea.getBottom()),
               2.0f);
}

