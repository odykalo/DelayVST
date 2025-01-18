/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CircularBufferDelayAudioProcessorEditor::CircularBufferDelayAudioProcessorEditor(CircularBufferDelayAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    //feedback slider code
    feedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(feedbackSlider);
    feedbackSlider.setRange(0.0f, 0.99f, 0.01f);
    feedbackSlider.setValue(audioProcessor.getAPVTS().getParameter("feedback")->getValue());
    feedbackSlider.onValueChange = [this]() {
        audioProcessor.getAPVTS().getParameter("feedback")->setValueNotifyingHost(feedbackSlider.getValue());
        };
    feedbackSlider.setSkewFactor(0.5);

    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.attachToComponent(&feedbackSlider, true);
    addAndMakeVisible(feedbackLabel);

    setSize(400, 300);



    // Load custom font
    juce::File fontFile("C:/Users/O-Stacks/AppData/Local/Microsoft/Windows/Fonts/papyrus.ttf");
    std::unique_ptr<juce::FileInputStream> fontStream(fontFile.createInputStream());

    if (fontStream != nullptr && fontStream->openedOk())
    {
        fontData = std::make_unique<juce::MemoryBlock>();
        fontStream->readIntoMemoryBlock(*fontData);
        papyrusTypeface = juce::Typeface::createSystemTypefaceFor(fontData->getData(), fontData->getSize());
    }
}

CircularBufferDelayAudioProcessorEditor::~CircularBufferDelayAudioProcessorEditor()
{
}

//==============================================================================
void CircularBufferDelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Title at the top
    if (papyrusTypeface != nullptr)
    {
        g.setFont(juce::Font(papyrusTypeface).withHeight(40.0f));  // Reduced height for better balance
    }
    else
    {
        g.setFont(24.0f);
    }
    g.setColour(juce::Colours::white);
    g.drawText("Odyssey Delay", 0, 20, getWidth(), 40, juce::Justification::centredTop);
}

void CircularBufferDelayAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Define a margin below the title
    int titleMargin = 60; // Adjust this value to control the spacing

    // Calculate the area for the feedback slider
    int sliderWidth = bounds.getWidth() / 3;
    int sliderHeight = bounds.getHeight() / 2; // Or whatever height you want

    int x = bounds.getCentreX() - sliderWidth / 2;
    int y = titleMargin; // Position below the title

    feedbackSlider.setBounds(x, y, sliderWidth, sliderHeight);
}
