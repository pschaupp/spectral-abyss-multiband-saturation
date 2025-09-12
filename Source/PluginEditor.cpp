/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
using namespace std;
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SaturationAudioProcessorEditor::SaturationAudioProcessorEditor (SaturationAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
        spectrumAnalyser(audioProcessor),
        amountLowAttachment(audioProcessor.getState(), "saturationAmountLow", saturationSliderLow),
        amountMidAttachment(audioProcessor.getState(), "saturationAmountMid", saturationSliderMid),
        amountHighAttachment(audioProcessor.getState(), "saturationAmountHigh", saturationSliderHigh),
        crossoverLowMidAttachment(audioProcessor.getState(), "crossoverLowMid", crossoverSliderLowMid),
        crossoverMidHighAttachment(audioProcessor.getState(), "crossoverMidHigh", crossoverSliderMidHigh)


{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 500);
    juce::LookAndFeel::setDefaultLookAndFeel (&customLookAndFeel);

    spectrumAnalyser.setAPVTSReference(audioProcessor.getState());

    saturationSliderLow.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    saturationSliderLow.setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    saturationSliderMid.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    saturationSliderMid.setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    saturationSliderHigh.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    saturationSliderHigh.setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);

    crossoverSliderLowMid.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    crossoverSliderLowMid.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    crossoverSliderLowMid.textFromValueFunction = [] (double value)
    {
        return juce::String (juce::roundToInt (value)) + "Hz";
    };
    crossoverSliderLowMid.updateText();

    crossoverSliderMidHigh.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    crossoverSliderMidHigh.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    crossoverSliderMidHigh.textFromValueFunction = [] (double value)
    {
        return juce::String (juce::roundToInt (value)) + "Hz";
    };
    crossoverSliderMidHigh.updateText();

    addAndMakeVisible(saturationSliderLow);
    addAndMakeVisible(saturationSliderMid);
    addAndMakeVisible(saturationSliderHigh);
    addAndMakeVisible(crossoverSliderLowMid);
    addAndMakeVisible(crossoverSliderMidHigh);

    saturationLabelLow.setText("Low", juce::dontSendNotification);
    saturationLabelLow.setJustificationType(juce::Justification::centred);
    saturationLabelMid.setText("Mid", juce::dontSendNotification);
    saturationLabelMid.setJustificationType(juce::Justification::centred);
    saturationLabelHigh.setText("High", juce::dontSendNotification);
    saturationLabelHigh.setJustificationType(juce::Justification::centred);
    crossoverLabelLow.setText("Crossover Low", juce::dontSendNotification);
    crossoverLabelLow.setJustificationType(juce::Justification::centred);
    crossoverLabelHigh.setText("Crossover High", juce::dontSendNotification);
    crossoverLabelHigh.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(saturationLabelLow);
    addAndMakeVisible(saturationLabelMid);
    addAndMakeVisible(saturationLabelHigh);
    addAndMakeVisible(crossoverLabelLow);
    addAndMakeVisible(crossoverLabelHigh);

    addAndMakeVisible(spectrumAnalyser);

    saturationTypeMenu.addItem("Tanh", 1);
    saturationTypeMenu.addItem("Tape", 2);
    addAndMakeVisible(saturationTypeMenu);

    saturationTypeMenuAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
    audioProcessor.getState(), "saturationType", saturationTypeMenu);



}

SaturationAudioProcessorEditor::~SaturationAudioProcessorEditor()
{
}

//==============================================================================
void SaturationAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colour(0xFF2D3640));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
}

void SaturationAudioProcessorEditor::resized()
{
    const int xCenter = getWidth() / 2;
    const int yCenter = getHeight() / 2;

    saturationLabelLow.setBounds (75, yCenter+120, 100, 20);
    saturationLabelMid.setBounds (xCenter-50, yCenter+120, 100, 20);
    saturationLabelHigh.setBounds (getWidth()-175, yCenter+120, 100, 20);
    saturationSliderLow.setBounds (60, yCenter+20, 130, 130);
    saturationSliderMid.setBounds (xCenter-65, yCenter+20, 130, 130);
    saturationSliderHigh.setBounds (getWidth()-190, yCenter+20, 130, 130);

    const int crossoverLowX = (saturationSliderMid.getX() - saturationSliderLow.getX())/2 + saturationSliderLow.getX() + 15;
    const int crossoverHighX = (saturationSliderHigh.getX() - saturationSliderMid.getX())/2 + saturationSliderMid.getX() + 15;

    crossoverLabelLow.setBounds (crossoverLowX, yCenter+100, 100, 20);
    crossoverLabelHigh.setBounds (crossoverHighX, yCenter+100, 100, 20);
    crossoverSliderLowMid.setBounds (crossoverLowX, yCenter+120, 100, 100);
    crossoverSliderMidHigh.setBounds (crossoverHighX, yCenter+120, 100, 100);

    spectrumAnalyser.setBounds (10, 0, getWidth()-20, 250);

    saturationTypeMenu.setBounds(getWidth()/2-50, 10, 100, 20);
}


