/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ProcessBLockAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ProcessBLockAudioProcessorEditor (SmudgeAudioProcessor&);
    ~ProcessBLockAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::ImageComponent mImageComponent;
    
    SmudgeAudioProcessor& audioProcessor;
    
    // IR
    juce::TextButton loadBtn;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::Label irName;
    
//    juce::ComboBox saturationChoice;
//    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> satChoiceAttach;
    
    juce::Slider driveSlider;
    juce::Label driveLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveSliderAttach;
    
    juce::Slider mixSlider;
    juce::Label mixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixSliderAttach;
    
    juce::Slider convoSlider;
    juce::Label convoLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> convoSliderAttach;
    
    juce::Slider dryGainSlider;
    juce::Label dryGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryGainSliderAttach;
 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessBLockAudioProcessorEditor)
};
