/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SmudgeAudioProcessor  : public juce::AudioProcessor, juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SmudgeAudioProcessor();
    ~SmudgeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Value Tree =======================================================================
    juce::AudioProcessorValueTreeState treeState;
    
    juce::dsp::Convolution irLoader;
    juce::File root, savedFile;
    
    
    void process(juce::dsp::ProcessContextReplacing<float> context);
    void updateParameters();
    
private:
    juce::dsp::ProcessSpec spec;
    juce::AudioBuffer<float> dryBuffer;
    
    float rawDrive = 1.0f;
    float rawMix = 0.5f;
    float rawConvo = 0.1f;
    float rawDryGain { 0.1f };
    
    bool phase = false;
    
    static constexpr float piDivisor = 2.0f / juce::MathConstants<float>::pi;

    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    //    juce::dsp::Oversampling<float> oversamplingModule;
    
//    bool osToggle {false};
    float softClipper(float samples);
    float hardClipper(float samples);
    
    void convolve(juce::dsp::ProcessContextReplacing<float> context);
    
    
    juce::dsp::Gain<float> convoGain;
    juce::dsp::Gain<float> dryGain;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmudgeAudioProcessor)
};
