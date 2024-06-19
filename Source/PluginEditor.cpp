/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <string>

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 600);
    auto& params = processor.getParameters();
    juce::AudioParameterFloat* audioParam = (juce::AudioParameterFloat*)params.getUnchecked(0);
    WGainSlider.setBounds(0, 355, 100, 210);
    WGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    WGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    WGainSlider.setRange(audioParam->range.start, audioParam->range.end);
    WGainSlider.setValue(*audioParam);
    WGainSlider.setColour(WGainSlider.textBoxTextColourId, juce::Colours::black);
    WGainSlider.addListener(this);
    addAndMakeVisible(WGainSlider);
    juce::AudioParameterFloat* audioParam1 = (juce::AudioParameterFloat*)params.getUnchecked(1);
    DGainSlider.setBounds(100, 355, 100, 210);
    DGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    DGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    DGainSlider.setRange(audioParam1->range.start, audioParam1->range.end);
    DGainSlider.setValue(*audioParam1);
    DGainSlider.setColour(DGainSlider.textBoxTextColourId, juce::Colours::black);
    DGainSlider.addListener(this);
    addAndMakeVisible(DGainSlider);
    juce::AudioParameterFloat* audioParam2 = (juce::AudioParameterFloat*)params.getUnchecked(2);
    FCSlider.setBounds(50, 160, 100, 100);
    FCSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    FCSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    FCSlider.setRange(audioParam2->range.start, audioParam2->range.end);
    FCSlider.setValue(*audioParam2);
    FCSlider.addListener(this);
    FCSlider.setColour(FCSlider.textBoxTextColourId, juce::Colours::black);
    addAndMakeVisible(FCSlider);
    juce::AudioParameterFloat* audioParam3 = (juce::AudioParameterFloat*)params.getUnchecked(3);
    fbSlider.setBounds(0, 25, 100, 100);
    fbSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    fbSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    fbSlider.setRange(audioParam3->range.start, audioParam3->range.end);
    fbSlider.setValue(*audioParam3);
    fbSlider.addListener(this);
    fbSlider.setColour(fbSlider.textBoxTextColourId, juce::Colours::black);
    addAndMakeVisible(fbSlider);
    juce::AudioParameterFloat* audioParam4 = (juce::AudioParameterFloat*)params.getUnchecked(4);
    decaySlider.setBounds(100, 25, 100, 100);
    decaySlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    decaySlider.setRange(audioParam4->range.start, audioParam4->range.end);
    decaySlider.setValue(*audioParam4);
    decaySlider.addListener(this);
    decaySlider.setColour(decaySlider.textBoxTextColourId, juce::Colours::black);
    addAndMakeVisible(decaySlider);
    DGainLabel.setText ("Dry Gain (dB)", juce::dontSendNotification);
    DGainLabel.setJustificationType(juce::Justification::centred);
    DGainLabel.attachToComponent(&DGainSlider, false);
    DGainLabel.setColour(DGainLabel.textColourId, juce::Colours::black);
    addAndMakeVisible (DGainLabel);
    WGainLabel.setText ("Wet Gain (dB)", juce::dontSendNotification);
    WGainLabel.setJustificationType(juce::Justification::centred);
    WGainLabel.attachToComponent(&WGainSlider, false);
    WGainLabel.setColour(WGainLabel.textColourId, juce::Colours::black);
    addAndMakeVisible (WGainLabel);
    FCLabel.setText ("High Cut (Hz)", juce::dontSendNotification);
    FCLabel.setJustificationType(juce::Justification::centred);
    FCLabel.attachToComponent(&FCSlider, false);
    FCLabel.setColour(FCLabel.textColourId, juce::Colours::black);
    addAndMakeVisible (FCLabel);
    decayLabel.setText ("Decay Time (ms)", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centred);
    decayLabel.attachToComponent(&decaySlider, false);
    decayLabel.setColour(decayLabel.textColourId, juce::Colours::black);
    addAndMakeVisible (decayLabel);
    fbLabel.setText ("Feedback Gain (dB)", juce::dontSendNotification);
    fbLabel.setJustificationType(juce::Justification::centred);
    fbLabel.attachToComponent(&fbSlider, false);
    fbLabel.setColour(fbLabel.textColourId, juce::Colours::black);
    addAndMakeVisible (fbLabel);
    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centredBottom);
    bpmLabel.attachToComponent(&bpm, false);
    bpmLabel.setColour(bpmLabel.textColourId, juce::Colours::black);
    addAndMakeVisible (bpmLabel);
    bpm.setBounds(15, 285, 50, 30);
    bpm.setColour(bpm.backgroundColourId, juce::Colours::grey);
    addAndMakeVisible(bpm);
    quater.setButtonText("1/4");
    quater.setBounds(75, 285, 50, 30);
    quater.onClick = [&]()
    {
        if(bpm.getText().toStdString() != ""){
            decaySlider.setValue(60/stof(bpm.getText().toStdString())*1000);
        }
    };
    quater.setColour(quater.textColourOffId, juce::Colours::black);
    quater.setColour(quater.buttonColourId, juce::Colours::grey);
    addAndMakeVisible(quater);
    half.setButtonText("1/2");
    half.setBounds(135, 285, 50, 30);
    half.onClick = [&]()
    {
        if(bpm.getText().toStdString() != ""){
            decaySlider.setValue(120/stof(bpm.getText().toStdString())*1000);
        }
    };
    half.setColour(half.textColourOffId, juce::Colours::black);
    half.setColour(half.buttonColourId, juce::Colours::grey);
    addAndMakeVisible(half);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::white);
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Mu45 Project 3", getLocalBounds(), juce::Justification::centredBottom, 1);
}

void NewProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

//void NewProjectAudioProcessorEditor::buttonClicked(juce::Button * button)
//{
//    if(bpm.getText() != ""){
//        auto& params = processor.getParameters();
//        juce::AudioParameterFloat* audioParam4 = (juce::AudioParameterFloat*)params.getUnchecked(4);
//        if(button == &quater)
//        {
//            *audioParam4 = 60/stof(bpm.getText().toStdString()); // set the param
//        }
//        else if(button == &half)
//        {
//            *audioParam4 = 120/stof(bpm.getText().toStdString()); // set the param
//        }
//    }
//}

void NewProjectAudioProcessorEditor::sliderValueChanged(juce::Slider * slider)
{
    auto& params = processor.getParameters();
    if (slider == &WGainSlider)
    {
        juce::AudioParameterFloat* audioParam = (juce::AudioParameterFloat*)params.getUnchecked(0);
        *audioParam = WGainSlider.getValue(); // set the AudioParameter
        DBG("Wet Gain Slider Changed");
    }
    else if (slider == &DGainSlider)
    {
        juce::AudioParameterFloat* audioParam1 = (juce::AudioParameterFloat*)params.getUnchecked(1);
        *audioParam1 = DGainSlider.getValue(); // set the param
        DBG("Dry Gain Slider Changed");
    }
    else if (slider == &FCSlider)
    {
        juce::AudioParameterFloat* audioParam2 = (juce::AudioParameterFloat*)params.getUnchecked(2);
        *audioParam2 = FCSlider.getValue(); // set the param
        DBG("FC Slider Changed");
    }
    else if (slider == &fbSlider)
    {
        juce::AudioParameterFloat* audioParam3 = (juce::AudioParameterFloat*)params.getUnchecked(3);
        *audioParam3 = fbSlider.getValue(); // set the param
        DBG("Feedback Slider Changed");
    }
    else if (slider == &decaySlider)
    {
        juce::AudioParameterFloat* audioParam4 = (juce::AudioParameterFloat*)params.getUnchecked(4);
        *audioParam4 = decaySlider.getValue(); // set the param
        DBG("Decay Slider Changed");
    };
}

