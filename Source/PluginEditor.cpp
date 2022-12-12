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
    
    // IR Loader
    
    addAndMakeVisible(loadBtn);
    loadBtn.setButtonText("Select Color");
    loadBtn.onClick = [this]()
    {
        fileChooser = std::make_unique<juce::FileChooser>("Choose File", audioProcessor.root, "*");
        
        const auto fileChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectDirectories;
        
        fileChooser->launchAsync(fileChooserFlags, [this](const juce::FileChooser& chooser)
         {
            juce::File result (chooser.getResult());
            
            if (result.getFileExtension() == ".wav" | result.getFileExtension() == ".mp3")
            {
                audioProcessor.savedFile = result;
                audioProcessor.root = result.getParentDirectory().getFullPathName();
                audioProcessor.irLoader.reset();
                audioProcessor.irLoader.loadImpulseResponse(result, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, 0); // 0 to initialise the size of the loaded file dynamically!
                irName.setText(result.getFileName(), juce::dontSendNotification);
            }
        });
    };
    
    addAndMakeVisible(irName);
    irName.setText("Color", juce::dontSendNotification);
    
    // Combobox
//    addAndMakeVisible(saturationChoice);
//    satChoiceAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.treeState, "saturationChoice", saturationChoice);
//    saturationChoice.setColour(juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::transparentWhite);
//
//    saturationChoice.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colour::fromString ("FF179595"));
//    saturationChoice.setColour(juce::ComboBox::ColourIds::arrowColourId, juce::Colour::fromString ("FF179595"));
//    saturationChoice.setText("Choose Smudge");
//    saturationChoice.setColour(juce::ComboBox::ColourIds::textColourId, juce::Colour::fromString ("FF179595"));
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
    
    addAndMakeVisible(convoSlider);
    convoSlider.setTextValueSuffix("dB");
    convoSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 64, 32);
    convoSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::transparentWhite);
    convoSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "convolution", convoSlider);

    addAndMakeVisible(convoLabel);
    convoLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    convoLabel.setText("Wet Level", juce::dontSendNotification);
    
    addAndMakeVisible(dryGainSlider);
    dryGainSlider.setTextValueSuffix("dB");
    dryGainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, true, 64, 32);
    dryGainSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::transparentWhite);
    dryGainSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "dryGain", dryGainSlider);

    addAndMakeVisible(dryGainLabel);
    dryGainLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    dryGainLabel.setText("Dry Level", juce::dontSendNotification);
    
    
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
    
    auto posX = getWidth() * .25;
    auto w = getWidth();
    auto h = getHeight();
    
    // IR
    loadBtn.setBounds(w * .35, 70, w * .3, 30);
    irName.setBounds(w * .35 + loadBtn.getWidth(), 70, w * .3, 30);
   
    
    // Sliders
    driveSlider.setBounds(posX + 70, h * .75, w * .4f, 35);
    driveLabel.setBounds(posX, h * .75, 65, 35);
    
    mixSlider.setBounds(posX + 70, h * .75 + 35, w * .4f, 35);
    mixLabel.setBounds(posX, h * .75 + 35, 65, 35);
    
    convoSlider.setBounds(posX + 70, h * .75 + 70, w * .4f, 35);
    convoLabel.setBounds(posX, h * .75 + 70, 65, 35);
    
    dryGainSlider.setBounds(posX + 70, h * .75 + 105, w * .4f, 35);
    dryGainLabel.setBounds(posX, h * .75 + 105, 65, 35);
    
//    saturationChoice.setBounds(w * .35, 30, w * .3, 30);
}
