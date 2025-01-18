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
class CircularBufferDelayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    CircularBufferDelayAudioProcessorEditor(CircularBufferDelayAudioProcessor&);
    ~CircularBufferDelayAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    CircularBufferDelayAudioProcessor& audioProcessor;

    //feedback slider
    juce::Slider feedbackSlider;
    juce::Label feedbackLabel;

    

    //custom font setup
    juce::Typeface::Ptr papyrusTypeface;
    std::unique_ptr<juce::MemoryBlock> fontData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircularBufferDelayAudioProcessorEditor)
};
