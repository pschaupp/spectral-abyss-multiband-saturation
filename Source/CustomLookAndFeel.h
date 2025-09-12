#pragma once

class CustomLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        constexpr unsigned lilac = 0xFFBAB0DC;
        constexpr unsigned lilacBright = 0xFFD3D0F2;
        constexpr unsigned shadow = 0xFF22272E;


        setColour(juce::Slider::thumbColourId, juce::Colour(lilac));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(lilacBright));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(shadow));
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::aliceblue);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

        setColour(juce::ComboBox::backgroundColourId, juce::Colour(shadow));
        setColour(juce::ComboBox::textColourId, juce::Colours::aliceblue);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);

        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(shadow));

        setColour(juce::Label::textColourId, juce::Colours::aliceblue);

    }
};
