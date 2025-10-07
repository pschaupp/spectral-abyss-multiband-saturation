#pragma once
#include "PluginProcessor.h"
#include "PluginEditor.h"

SaturationAudioProcessor::SaturationAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), apvts(*this, nullptr, "parameters", createParameters())
{
    tanhSaturationLow = std::make_unique<TanhSaturation>();
    tanhSaturationMid = std::make_unique<TanhSaturation>();
    tanhSaturationHigh = std::make_unique<TanhSaturation>();

    tapeSaturationLow = std::make_unique<TapeSaturation>();
    tapeSaturationMid = std::make_unique<TapeSaturation>();
    tapeSaturationHigh = std::make_unique<TapeSaturation>();

}

SaturationAudioProcessor::~SaturationAudioProcessor() {
}

//==============================================================================
const juce::String SaturationAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool SaturationAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SaturationAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SaturationAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SaturationAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int SaturationAudioProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SaturationAudioProcessor::getCurrentProgram() {
    return 0;
}

void SaturationAudioProcessor::setCurrentProgram(int index) {
}

const juce::String SaturationAudioProcessor::getProgramName(int index) {
    return {};
}

void SaturationAudioProcessor::changeProgramName(int index, const juce::String &newName) {
}

//============================================================================ ==
void SaturationAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    const int channels = getTotalNumOutputChannels();

    tanhSaturationLow->prepare(sampleRate, samplesPerBlock, channels);
    tanhSaturationMid->prepare(sampleRate, samplesPerBlock, channels);
    tanhSaturationHigh->prepare(sampleRate, samplesPerBlock, channels);

    tapeSaturationLow->prepare(sampleRate, samplesPerBlock, channels);
    tapeSaturationMid->prepare(sampleRate, samplesPerBlock, channels);
    tapeSaturationHigh->prepare(sampleRate, samplesPerBlock, channels);

    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = channels;

    lowMidLower.prepare(spec);
    lowMidHigher.prepare(spec);
    midHighLower.prepare(spec);
    midHighHigher.prepare(spec);

    lowMidLower.reset();
    lowMidHigher.reset();
    midHighLower.reset();
    midHighHigher.reset();

    lowMidLower.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    lowMidHigher.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    midHighLower.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    midHighHigher.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    lowMidLower.setCutoffFrequency(apvts.getRawParameterValue("crossoverLowMid")->load());
    lowMidHigher.setCutoffFrequency(apvts.getRawParameterValue("crossoverLowMid")->load());
    midHighLower.setCutoffFrequency(apvts.getRawParameterValue("crossoverMidHigh")->load());
    midHighHigher.setCutoffFrequency(apvts.getRawParameterValue("crossoverMidHigh")->load());

    lowBuf.setSize(getTotalNumInputChannels(), samplesPerBlock);
    midBuf.setSize(getTotalNumInputChannels(), samplesPerBlock);
    highBuf.setSize(getTotalNumInputChannels(), samplesPerBlock);

}

void SaturationAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SaturationAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void SaturationAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();


    const int numSamples = buffer.getNumSamples();
    if (lowBuf.getNumSamples() != numSamples)
    {
        lowBuf.setSize(totalNumOutputChannels, numSamples, false, false, true);
        midBuf.setSize(totalNumOutputChannels, numSamples, false, false, true);
        highBuf.setSize(totalNumOutputChannels, numSamples, false, false, true);
    }


    const float saturationTypeChoice = apvts.getRawParameterValue("saturationType")->load();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    const float lowMidFreq = apvts.getRawParameterValue("crossoverLowMid")->load();
    const float midHighFreq = apvts.getRawParameterValue("crossoverMidHigh")->load();

    lowMidLower.setCutoffFrequency(lowMidFreq);
    lowMidHigher.setCutoffFrequency(lowMidFreq);
    midHighLower.setCutoffFrequency(midHighFreq);
    midHighHigher.setCutoffFrequency(midHighFreq);


    const float saturationAmountLow = apvts.getRawParameterValue("saturationAmountLow")->load();
    const float saturationAmountMid = apvts.getRawParameterValue("saturationAmountMid")->load();
    const float saturationAmountHigh = apvts.getRawParameterValue("saturationAmountHigh")->load();

    const juce::dsp::AudioBlock<float> inBlock(buffer);
    juce::dsp::AudioBlock<float> lowBlock(lowBuf);
    juce::dsp::AudioBlock<float> midBlock(midBuf);
    juce::dsp::AudioBlock<float> highBlock(highBuf);


    lowMidLower.process(juce::dsp::ProcessContextNonReplacing<float>(inBlock, lowBlock));
    lowMidHigher.process(juce::dsp::ProcessContextNonReplacing<float>(inBlock, midBlock));

    midHighLower.process(juce::dsp::ProcessContextReplacing<float>(midBlock));
    midHighHigher.process(juce::dsp::ProcessContextNonReplacing<float>(inBlock, highBlock));



    if (saturationTypeChoice == 0) {
        tanhSaturationLow->processBlock(lowBlock, saturationAmountLow, totalNumInputChannels);
        tanhSaturationMid->processBlock(midBlock, saturationAmountMid, totalNumInputChannels);
        tanhSaturationHigh->processBlock(highBlock, saturationAmountHigh, totalNumInputChannels);
    }
    else {
        tapeSaturationLow->processBlock(lowBlock, saturationAmountLow, totalNumInputChannels);
        tapeSaturationMid->processBlock(midBlock, saturationAmountMid, totalNumInputChannels);
        tapeSaturationHigh->processBlock(highBlock, saturationAmountHigh, totalNumInputChannels);
    }

    for (size_t ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* dst  = buffer.getWritePointer(ch);
        const float* low = lowBuf.getReadPointer(ch);
        const float* mid = midBuf.getReadPointer(ch);
        const float* high = highBuf.getReadPointer(ch);

        for (int n = 0; n < buffer.getNumSamples(); ++n)
        {
            dst[n] = low[n] + mid[n] + high[n];
        }

        if (ch == 0) {
            auto* channelData = buffer.getReadPointer (0);
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                pushNextSampleIntoFifo(channelData[i]);
            }
        }
    }
}

void SaturationAudioProcessor::pushNextSampleIntoFifo (const float sample)
{
    int start1, size1, start2, size2;
    abstractFifo.prepareToWrite (1, start1, size1, start2, size2);

    if (size1 > 0)
        fifoBuffer[start1] = sample;

    abstractFifo.finishedWrite (1);
}


//==============================================================================
bool SaturationAudioProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor *SaturationAudioProcessor::createEditor()
{
    return new SaturationAudioProcessorEditor (*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SaturationAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SaturationAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor * JUCE_CALLTYPE createPluginFilter() {
    return new SaturationAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout SaturationAudioProcessor::createParameters() {
    return {
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"saturationAmountLow"},
                                                    "Amount Low",
                                                    1.0f,
                                                    40.0f,
                                                    1.0f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"saturationAmountMid"},
                                                    "Amount Mid",
                                                    1.0f,
                                                    40.0f,
                                                    1.0f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"saturationAmountHigh"},
                                                    "Amount High",
                                                    1.0f,
                                                    40.0f,
                                                    1.0f),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"saturationType"},
                                                    "Saturation Type",
                                                    juce::StringArray{"Tanh", "Tape"},
                                                    1),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"crossoverLowMid"},
                                                    "Crossover Low-Mid",
                                                    juce::NormalisableRange<float>(30.0f, 400.0f, 1.0f, 0.6f),
                                                    150.0f
                                                    ),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"crossoverMidHigh"},
                                                    "Crossover Mid-High",
                                                    juce::NormalisableRange<float>(300.0f, 10000.0f, 1.0f, 0.6f),
                                                    1500.0f
                                                    )

    };
}
