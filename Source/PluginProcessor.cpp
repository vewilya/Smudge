/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SmudgeAudioProcessor::SmudgeAudioProcessor()
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
    treeState.addParameterListener("delay", this);
    treeState.addParameterListener("drive", this);
    treeState.addParameterListener("mix", this);
    treeState.addParameterListener("dryGain", this);
    treeState.addParameterListener("convolution", this);
}

SmudgeAudioProcessor::~SmudgeAudioProcessor()
{
    treeState.removeParameterListener("delay", this);
    treeState.removeParameterListener("drive", this);
    treeState.removeParameterListener("mix", this);
    treeState.removeParameterListener("dryGain", this);
    treeState.removeParameterListener("convolution", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout SmudgeAudioProcessor::createParameterLayout()
{
    juce::StringArray satChoices = { "Soft", "Hard" };
    
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
//    auto pChoice = std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "saturationChoice",  1 }, "SaturationChoice", juce::StringArray ("soft clip", "hard clip") , 0);
    auto pDrive = std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "drive",  1 }, "Drive",  0.0f, 1.0f, 0.2f );
    auto pMix   = std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "mix", 1 }, "Mix",  0.0f, 100.0f, 50.0f );
    
//    auto pDelay = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "delay", 1}, "Delay", 1.f, 44100.0f, 0.0f );
    auto pConvo = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "convolution", 1}, "Convolution", -96.0f, 0.0f, -20.0f );
    auto pDryGain = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "dryGain", 1}, "Dry Gain", -96.0f, 0.0f, -20.0f );
    
//    params.push_back(std::move(pChoice));
//    params.push_back(std::move(pDelay));
    params.push_back(std::move(pDrive));
    params.push_back(std::move(pMix));
    params.push_back(std::move(pConvo));
    params.push_back(std::move(pDryGain));

    return { params.begin(), params.end() };
}

void SmudgeAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (parameterID == "drive")
    {
        rawDrive = newValue * 5.0f + 2.0f;
//        DBG("Drive is " <<rawDrive);
    }
    
    if (parameterID == "mix")
    {
        rawMix = newValue * 0.01f;
    }
    
    if (parameterID == "convolution")
    {
        rawConvo = newValue;
    }
    
    if (parameterID == "dryGain")
    {
        rawDryGain = newValue;
    }
}

//==============================================================================
const juce::String SmudgeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SmudgeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SmudgeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SmudgeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SmudgeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SmudgeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SmudgeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SmudgeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SmudgeAudioProcessor::getProgramName (int index)
{
    return {};
}

void SmudgeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SmudgeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
     
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    // Passing that to the dry buffer copy
    dryBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
    
    convoGain.reset();
    convoGain.prepare(spec);
    
    dryGain.reset();
    dryGain.prepare(spec);
    
    irLoader.reset();
    irLoader.prepare(spec);

    rawDrive = *treeState.getRawParameterValue("drive") * 5.0f + 2.0f;
    rawMix = *treeState.getRawParameterValue("mix") * 0.01f;
    rawConvo = *treeState.getRawParameterValue("convolution");
    rawDryGain = *treeState.getRawParameterValue("dryGain");
}

void SmudgeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SmudgeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SmudgeAudioProcessor::updateParameters()
{
    // update your Parameters for your procceses here
}

//void SmudgeAudioProcessor::process(juce::dsp::ProcessContextReplacing<float> context)
//{
//    // do processing here and output
//}

void SmudgeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    dryBuffer.makeCopyOf(buffer, true);
    juce::dsp::AudioBlock<float> dryBlock {dryBuffer};
    juce::dsp::AudioBlock<float> block {buffer};

    
    for (int sample = 0; sample < block.getNumSamples(); sample++)
    {
        for(int channel = 0; channel < block.getNumChannels(); channel++)
        {
            auto* channelData = block.getChannelPointer(channel);

            const auto input = channelData[sample];

            const auto sat = softClipper(input);
            const auto satMix = input * (1.0f - rawMix) + rawMix * sat;
            
            channelData[sample] = satMix;
        }
    }
    
    for (int sample = 0; sample < dryBlock.getNumSamples(); sample++)
    {
        for(int channel = 0; channel < block.getNumChannels(); channel++)
        {
            auto* channelData = dryBlock.getChannelPointer(channel);

            const auto dryIn = channelData[sample];

            const auto drySat = hardClipper(dryIn);
            const auto drySatMix = dryIn * (1.0f - rawMix) + rawMix * drySat;
            
            channelData[sample] = drySatMix;
        }
    }
    
    convolve(juce::dsp::ProcessContextReplacing<float> (block));
    
    auto wetContext = juce::dsp::ProcessContextReplacing<float> (block);
    auto dryContext = juce::dsp::ProcessContextReplacing<float> (dryBlock);
    
    convoGain.setGainDecibels(rawConvo);
    convoGain.process(wetContext);
    
    dryGain.setGainDecibels(rawDryGain);
    dryGain.process(dryContext);
    
    wetContext.getOutputBlock().add(dryBlock);
}

void SmudgeAudioProcessor::convolve(juce::dsp::ProcessContextReplacing<float> context)
{
    if (irLoader.getCurrentIRSize() > 0)
    {
        irLoader.process(juce::dsp::ProcessContextReplacing<float>(context));
    }
}


float SmudgeAudioProcessor::softClipper(float samples)
{
    const auto unity = 1.0 - ( rawDrive / 14.0f);
    const auto sat = piDivisor * std::atanf(rawDrive * samples) * unity;
    
    return sat;
}

float SmudgeAudioProcessor::hardClipper(float samples)
{
    samples *= rawDrive;
    
    if (std::abs(samples) > 1.0)
    {
        samples *= 1.0 / std::abs(samples);
    }
    
    return samples;
}

//==============================================================================
bool SmudgeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SmudgeAudioProcessor::createEditor()
{
    return new ProcessBLockAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void SmudgeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    //Save Param
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream (stream);
}

void SmudgeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new SmudgeAudioProcessor();
}
