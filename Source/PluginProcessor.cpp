/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(wetGain = new juce::AudioParameterFloat("wGain", // parameterID,
    "Wet Gain", // parameterName,
    -50.0f, // minValue,
    2.0f, // maxValue,
    -6.0f)); // defaultValue
    // Filter Q parameter
    addParameter(dryGain = new juce::AudioParameterFloat("dGain", // parameterID,
    "Dry Gain", // parameterName,
    -10.0f, // minValue,
    2.0f, // maxValue,
    0.0f)); // defaultValue
    addParameter(LFilterFcParam = new juce::AudioParameterFloat("LPfc", // parameterID,
    "Low Pass Fc", // parameterName,
    1000.0f, // minValue,
    22000.0f, // maxValue,
    22000.0f)); // defaultValue
    addParameter(feedbackGain = new juce::AudioParameterFloat("fbGain", // parameterID,
    "Feedback Gain", // parameterName,
    -20.0f, // minValue,
    0.0f, // maxValue,
    -4.0f)); // defaultValue
    addParameter(delayTime = new juce::AudioParameterFloat("decayTime", // parameterID,
    "Decay Time", // parameterName,
    0.0f, // minValue,
    5000.0f, // maxValue,
    40.0f)); // defaultValue
}

int NewProjectAudioProcessor::calcDelaySampsFromMsec(float time){
    return (time * mFs) / 1000;
}

void NewProjectAudioProcessor::calcDelays()
{
       float msec = delayTime->get();
       int samps = calcDelaySampsFromMsec(msec);
       mDelay1L.setDelay(samps);
       mDelay1R.setDelay(samps);
}

void NewProjectAudioProcessor::calcGains()
{
    mWetGain = wetGain->get();
    mDryGain = dryGain->get();
    mWetGain = powf(10.0, mWetGain*0.05);
    mDryGain = powf(10.0, mDryGain*0.05);
    mFeedbackGainLin = powf(10.0, feedbackGain->get()*0.05);
}

void NewProjectAudioProcessor::calcFilterCoeffs(){
    float coeffs[5];
    Mu45FilterCalc::calcCoeffsLPF(coeffs, LFilterFcParam->get(), 0.707, mFs);
    filterL.setCoefficients(coeffs[0], coeffs[1], coeffs[2], coeffs[3], coeffs[4]);
    filterR.setCoefficients(coeffs[0], coeffs[1], coeffs[2], coeffs[3], coeffs[4]);
}

void NewProjectAudioProcessor::calcAlgorithmParams(){
    calcGains();
    calcDelays();
    calcFilterCoeffs();
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // Get system sample rate
    mFs = sampleRate;
    // Initialize your Delay objects with maximum length
    int maxSamps = calcDelaySampsFromMsec(5000);
    mDelay1L.setMaximumDelay(maxSamps);
    mDelay1R.setMaximumDelay(maxSamps);
    calcAlgorithmParams();
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    // Do the guts of your plugin's audio processing...
    const int numSamples = buffer.getNumSamples();
    // This version assumes we always have 2 channels
    calcAlgorithmParams();
    float* channelDataL = buffer.getWritePointer (0);
    float* channelDataR = buffer.getWritePointer (1);
    float tempL, tempR;
    // The "inner loop"
    for (int i = 0; i < numSamples; ++i)
    {
        // get next out
        tempL = mDelay1L.nextOut();
        tempR = mDelay1R.nextOut();
        // compute next input to filter (with feedback) and tick
        mDelay1L.tick(mWetGain * channelDataL[i] + mFeedbackGainLin * tempL + filterL.tick(channelDataL[i]));
        mDelay1R.tick(mWetGain * channelDataR[i] + mFeedbackGainLin * tempR + filterR.tick(channelDataR[i]));
        // mix wet and dry
        channelDataL[i] = mDryGain * channelDataL[i] + mWetGain * tempL;
        channelDataR[i] = mDryGain * channelDataR[i] + mWetGain * tempR;
    }
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
