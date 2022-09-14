/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ProcessBLockAudioProcessorEditor::ProcessBLockAudioProcessorEditor (SmudgeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // BG Image
    auto smudgeImage = juce::ImageCache::getFromMemory(BinaryData::Smudge_png, BinaryData::Smudge_pngSize);
    
    if(!smudgeImage.isNull())
        mImageComponent.setImage(smudgeImage, juce::RectanglePlacement::fillDestination);
        jassert(! smudgeImage.isNull());
    
    addAndMakeVisible(mImageComponent);
    
    // UI Size and Resizing
    setSize (500, 500);
    setResizable(true, true);
    getConstrainer()->setFixedAspectRatio(1.0);
    setResizeLimits(250, 250, 750, 750);
    
    // Combobox
    addAndMakeVisible(saturationChoice);
    satChoiceAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.treeState, "saturationChoice", saturationChoice);
    saturationChoice.setColour(juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::transparentWhite);
    
    saturationChoice.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colour::fromString ("FF179595"));
    saturationChoice.setColour(juce::ComboBox::ColourIds::arrowColourId, juce::Colour::fromString ("FF179595"));
    saturationChoice.setText("Choose Smudge");
    saturationChoice.setColour(juce::ComboBox::ColourIds::textColourId, juce::Colour::fromString ("FF179595"));
//    saturationChoice.setColour(juce::ComboBox::ColourIds::focusedOutlineColourId, juce::Colours::transparentWhite);
//    saturationChoice.setColour(juce::ComboBox::ColourIds::buttonColourId, juce::Colours::transparentWhite);
    
    // Drive Slider
    addAndMakeVisible(driveSlider);
    driveSlider.setTextValueSuffix("");
    driveSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 64, 32);
    driveSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::transparentWhite);
    driveSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "drive", driveSlider);
    
    addAndMakeVisible(driveLabel);
    driveLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    driveLabel.setText("Drive", juce::dontSendNotification);
    
    // Mix Slider
    addAndMakeVisible(mixSlider);
    mixSlider.setTextValueSuffix("%");
    mixSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 64, 32);
    mixSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::transparentWhite);
    mixSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "mix", mixSlider);
    
    addAndMakeVisible(mixLabel);
    mixLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    mixLabel.setText("Mix", juce::dontSendNotification);
    
}

ProcessBLockAudioProcessorEditor::~ProcessBLockAudioProcessorEditor()
{
}

//==============================================================================
void ProcessBLockAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
//background: linear-gradient(225deg, #654784, #179595);
    const juce::String colourString ("654784");
    const juce::Colour colour (juce::Colour::fromString ("FF" + colourString));
    
    const juce::String colourString2 ("179595");
    const juce::Colour colour2 (juce::Colour::fromString ("FF" + colourString2));
    
    
    g.setGradientFill(juce::ColourGradient::vertical(colour2, getHeight() * 0.9f, colour, getHeight() * 0.1f));
    g.fillAll();

}

void ProcessBLockAudioProcessorEditor::resized()
{
    
    // BG Image
    mImageComponent.setBoundsRelative(0.0f, 0.35f, 1.0f, 0.35f);
    
    // Sliders
    auto posX = getWidth() * .25;
    auto w = getWidth();
    auto h = getHeight();
    
    driveSlider.setBounds(posX + 70, h * .75, w * .4f, 35);
    driveLabel.setBounds(posX, h * .75, 65, 35);
    
    mixSlider.setBounds(posX + 70, h * .75 + 35, w * .4f, 35);
    mixLabel.setBounds(posX, h * .75 + 35, 65, 35);
    
    saturationChoice.setBounds(w * .35, 30, w * .3, 30);
}
