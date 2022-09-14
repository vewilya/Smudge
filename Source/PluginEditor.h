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
    
    juce::ComboBox saturationChoice;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> satChoiceAttach;
    
    juce::Slider driveSlider;
    juce::Label driveLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveSliderAttach;
    
    juce::Slider mixSlider;
    juce::Label mixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixSliderAttach;
 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessBLockAudioProcessorEditor)
};
