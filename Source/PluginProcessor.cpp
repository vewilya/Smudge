/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ProcessBLockAudioProcessor::ProcessBLockAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, "PARAMETERS",  createParameterLayout())
#endif
{
    treeState.addParameterListener("drive", this);
    treeState.addParameterListener("mix", this);
}

ProcessBLockAudioProcessor::~ProcessBLockAudioProcessor()
{
    treeState.removeParameterListener("drive", this);
    treeState.removeParameterListener("mix", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout ProcessBLockAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;

    auto pDrive = std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "drive",  1 }, "Drive",  1.0f, 7.0f, 1.0f );
    auto pMix   = std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "mix", 1 }, "Mix",  0.0f, 100.0f, 50.0f );
    
    params.push_back(std::move(pDrive));
    params.push_back(std::move(pMix));

    return { params.begin(), params.end() };
}

void ProcessBLockAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (parameterID == "drive")
    {
        rawDrive = newValue ;
//        DBG("Drive is " <<rawDrive);
    }
    
    if (parameterID == "mix")
    {
        rawMix = newValue * 0.01f;
    }
}

//==============================================================================
const juce::String ProcessBLockAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ProcessBLockAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ProcessBLockAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ProcessBLockAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ProcessBLockAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ProcessBLockAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ProcessBLockAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ProcessBLockAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ProcessBLockAudioProcessor::getProgramName (int index)
{
    return {};
}

void ProcessBLockAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ProcessBLockAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
//    rawDrive = juce::Decibels::decibelsToGain(static_cast<float>(*treeState.getRawParameterValue("drive")));
    rawDrive = *treeState.getRawParameterValue("drive");
    rawMix = *treeState.getRawParameterValue("mix") * 0.01f;
}

void ProcessBLockAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ProcessBLockAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
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

void ProcessBLockAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block (buffer);
    
    for (int sample = 0; sample < block.getNumSamples(); sample++)
    {
        for(int channel = 0; channel < block.getNumChannels(); channel++)
        {
            auto* channelData = block.getChannelPointer(channel);
             
            const auto input = channelData[sample];
            const auto unity = 1.0 - ( rawDrive / 30.0f);
            const auto sat = piDivisor * std::atanf(rawDrive * input) * unity;

            const auto mix = input * (1.0f - rawMix) + rawMix * sat;

            channelData[sample] = mix;
            
        }
    }
    
//    if (osToggle)
//    {
//        // Oversample
//    }
//
//    else
//    {
//        // regular behaviour
//    }
}

//==============================================================================
bool ProcessBLockAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ProcessBLockAudioProcessor::createEditor()
{
    return new ProcessBLockAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void ProcessBLockAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    //Save Param
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream (stream);
}

void ProcessBLockAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
   //Recall Param
    
    auto tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid())
    {
        treeState.state = tree;
        rawDrive = juce::Decibels::decibelsToGain(static_cast<float>(*treeState.getRawParameterValue("drive")));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProcessBLockAudioProcessor();
}
