/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LinkwitzRileySecondOrder::LinkwitzRileySecondOrder()
{
}

void LinkwitzRileySecondOrder::init(int sampleRate)
{
	m_SampleRate = sampleRate;
}

void LinkwitzRileySecondOrder::setFrequency(float frequency)
{
	if (m_SampleRate == 0)
	{
		return;
	}

	const float pi = 3.141592653589793f;

	const float fpi = pi * frequency;
	const float wc = 2.0f * fpi;
	const float wc2 = wc * wc;
	const float wc22 = 2.0f * wc2;
	const float k = wc / tanf(fpi / m_SampleRate);
	const float k2 = k * k;
	const float k22 = 2 * k2;
	const float wck2 = 2 * wc * k;
	const float tmpk = k2 + wc2 + wck2;

	m_b1 = (-k22 + wc22) / tmpk;
	m_b2 = (-wck2 + k2 + wc2) / tmpk;

	//---------------
	// low-pass
	//---------------
	m_a0_lp = wc2 / tmpk;
	m_a1_lp = wc22 / tmpk;
	m_a2_lp = wc2 / tmpk;

	//----------------
	// high-pass
	//----------------
	m_a0_hp = k2 / tmpk;
	m_a1_hp = -k22 / tmpk;
	m_a2_hp = k2 / tmpk;
}

float LinkwitzRileySecondOrder::processLP(float in)
{
	const float y0 = m_a0_lp * in + m_x0_lp;
	m_x0_lp = m_a1_lp * in - m_b1 * y0 + m_x1_lp;
	m_x1_lp = m_a2_lp * in - m_b2 * y0;

	return y0;
}

float LinkwitzRileySecondOrder::processHP(float in)
{
	const float y0 = m_a0_hp * in + m_x0_hp;
	m_x0_hp = m_a1_hp * in - m_b1 * y0 + m_x1_hp;
	m_x1_hp = m_a2_hp * in - m_b2 * y0;

	return -y0;
}

//==============================================================================
FirstOrderAllPass::FirstOrderAllPass()
{
}

void FirstOrderAllPass::init(int sampleRate)
{
	m_SampleRate = sampleRate;
}

void FirstOrderAllPass::setCoefrequencyParameter(float frequency)
{
	if (m_SampleRate == 0)
	{
		return;
	}

	const float tmp = tanf(3.14f * frequency / m_SampleRate);
	m_a1 = (tmp - 1.0f) / (tmp + 1.0f);
}

void FirstOrderAllPass::setCoef(float coef)
{
	m_a1 = coef;
}

float FirstOrderAllPass::process(float in)
{
	const float tmp = m_a1 * in + m_d;
	m_d = in - m_a1 * tmp;
	return tmp;
}

//==============================================================================

const std::string StereoEnhancerAudioProcessor::paramsNames[] = { "Intensity", "HPFilter", "LPFilter", "Width", "Volume" };

//==============================================================================
StereoEnhancerAudioProcessor::StereoEnhancerAudioProcessor()
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
	intensityParameter = apvts.getRawParameterValue(paramsNames[0]);
    hpFilterParameter  = apvts.getRawParameterValue(paramsNames[1]);
	lpFilterParameter  = apvts.getRawParameterValue(paramsNames[2]);
	widthParameter     = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[4]);

	buttonMonoParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonMono"));
}

StereoEnhancerAudioProcessor::~StereoEnhancerAudioProcessor()
{
}

//==============================================================================
const juce::String StereoEnhancerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StereoEnhancerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StereoEnhancerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StereoEnhancerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StereoEnhancerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StereoEnhancerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StereoEnhancerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StereoEnhancerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StereoEnhancerAudioProcessor::getProgramName (int index)
{
    return {};
}

void StereoEnhancerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StereoEnhancerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	int sr = (int)(sampleRate);
	
	for (int i = 0; i < N_ALL_PASS_FO; i++)
	{
		m_firstOrderAllPass[i].init(sr);
	}

	m_lowPassFilter.init(sr);
	m_highPassFilter.init(sr);
}

void StereoEnhancerAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StereoEnhancerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void StereoEnhancerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Buttons
	const auto buttonMono = buttonMonoParameter->get();

	// Get params
	const auto intensity = intensityParameter->load();
	const auto hpFilter = hpFilterParameter->load();
	const auto lpFilter = lpFilterParameter->load();
	const auto width = widthParameter->load();
	const auto volume = 0.5f * juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	if (channels < 2)
		return;

	// Channel pointer
	auto* leftChannelBuffer = buffer.getWritePointer(0);
	auto* rightChannelBuffer = buffer.getWritePointer(1);

	const float frequencyMinMel = FrequencyToMel(hpFilter);
	const float frequencyMaxMel = FrequencyToMel(lpFilter);
	const int count = int((0.1f + 0.9f * intensity) * N_ALL_PASS_FO);
	const float stepMel = (frequencyMaxMel - frequencyMinMel) / count;

	m_highPassFilter.setFrequency(hpFilter);
	m_lowPassFilter.setFrequency(lpFilter);

	for (int i = 0; i < count; i++)
	{
		m_firstOrderAllPass[i].setCoefrequencyParameter(MelToFrequency(frequencyMinMel + i * stepMel));
	}

	for (int sample = 0; sample < samples; ++sample)
	{
		// Get input
		const float inLeft = leftChannelBuffer[sample];
		const float inRight = rightChannelBuffer[sample];

		const float inMid = inLeft + inRight;
		const float inSide = inLeft - inRight;

		float inAllPass = inMid;

		for (int i = 0; i < count; i++)
		{
			inAllPass = m_firstOrderAllPass[i].process(inAllPass);
		}

		// Low pass + high pass filter
		inAllPass = m_lowPassFilter.processLP(inAllPass);
		inAllPass = m_highPassFilter.processHP(inAllPass);

		// Apply volume, width and send to output
		const float inAllPassWidth = inAllPass * width;
		const float outLeft = volume * (inMid + inSide + inAllPassWidth);
		const float outRight = volume * (inMid - inSide - inAllPassWidth);

		// Sum to mono
		if (buttonMono)
		{
			const float outSum = 0.5f * (outLeft + outRight);
			
			leftChannelBuffer[sample] = outSum;
			rightChannelBuffer[sample] = outSum;
		}
		else
		{
			leftChannelBuffer[sample] = volume * (inMid + inSide + inAllPassWidth);
			rightChannelBuffer[sample] = volume * (inMid - inSide - inAllPassWidth);
		}
	}
}

//==============================================================================
bool StereoEnhancerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StereoEnhancerAudioProcessor::createEditor()
{
    return new StereoEnhancerAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void StereoEnhancerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void StereoEnhancerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout StereoEnhancerAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(    0.0f,     1.0f, 0.01f, 1.0f),     0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   20.0f,   880.0f,  1.0f, 0.3f),    20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( 4000.0f, 20000.0f,  1.0f, 0.3f), 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(    0.0f,     1.0f, 0.01f, 1.0f),     0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(  -12.0f,    12.0f,  0.1f, 1.0f),     0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonMono", "ButtonMono", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoEnhancerAudioProcessor();
}
