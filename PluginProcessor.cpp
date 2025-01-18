/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CircularBufferDelayAudioProcessor::CircularBufferDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    apvts.addParameterListener("feedback", this); // Add listener for parameter changes
}

juce::AudioProcessorValueTreeState::ParameterLayout CircularBufferDelayAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    //feedback
    layout.add(std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", juce::NormalisableRange<float>(0.0f, 0.99f, 0.01f), 0.5f));
    //delay time
    layout.add(std::make_unique<juce::AudioParameterFloat>("delayTime", "Delay Time", juce::NormalisableRange<float>(0.0f, 6000.0f, 1.0f), 500.0f));
    return layout;
}

CircularBufferDelayAudioProcessor::~CircularBufferDelayAudioProcessor()
{
    apvts.removeParameterListener("feedback", this); // Remove listener when processor is deleted
}


//==============================================================================
const juce::String CircularBufferDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CircularBufferDelayAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool CircularBufferDelayAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool CircularBufferDelayAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double CircularBufferDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CircularBufferDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int CircularBufferDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CircularBufferDelayAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String CircularBufferDelayAudioProcessor::getProgramName(int index)
{
    return {};
}

void CircularBufferDelayAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void CircularBufferDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    auto delayBufferSize = (int)(sampleRate * 2.0);
    delayBuffer.setSize(getTotalNumOutputChannels(), (int)delayBufferSize);
    writePosition = 0;
}

void CircularBufferDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CircularBufferDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void CircularBufferDelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());



    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {

        fillBuffer(buffer, channel);
        readFromBuffer(buffer, delayBuffer, channel);
        fillBuffer(buffer, channel); 
    }
    updateBufferPositions(buffer, delayBuffer);
}

void CircularBufferDelayAudioProcessor::fillBuffer(juce::AudioBuffer<float>& buffer, int channel)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    for (int i = 0; i < bufferSize; ++i)
    {
        delayBuffer.setSample(channel, int(writePosition + i) % delayBufferSize, buffer.getSample(channel, i)); // Corrected line
    }
}

void CircularBufferDelayAudioProcessor::readFromBuffer(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer, int channel)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();
   
    //Get delay time in samples
    float delayTimeMs = apvts.getRawParameterValue("delayTime")->load();
    int delayTimeSamples = juce::jlimit(0, delayBufferSize -1, juce::roundToInt((delayTimeMs * getSampleRate()) / 1000.0f));
   
    //readPosition Calculation
    int readPosition = (writePosition - delayTimeSamples + delayBufferSize) % delayBufferSize;


    float g = apvts.getRawParameterValue("feedback")->load();

    for (int i = 0; i < bufferSize; ++i)
    {
        buffer.addSample(channel, i, delayBuffer.getSample(channel, int(readPosition + i) % delayBufferSize) * g);
    }
}

void CircularBufferDelayAudioProcessor::updateBufferPositions(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& delayBuffer)
{
    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = delayBuffer.getNumSamples();

    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}
void CircularBufferDelayAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "feedback")
    {
        // Do something when the feedback parameter changes (if needed)
        DBG("Feedback value changed to: " + juce::String(newValue));
    }
}

//==============================================================================
bool CircularBufferDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CircularBufferDelayAudioProcessor::createEditor()
{
    return new CircularBufferDelayAudioProcessorEditor (*this);
}

//==============================================================================
void CircularBufferDelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CircularBufferDelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CircularBufferDelayAudioProcessor();
}