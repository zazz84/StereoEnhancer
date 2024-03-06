/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StereoEnhancerAudioProcessorEditor::StereoEnhancerAudioProcessorEditor (StereoEnhancerAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	juce::Colour light  = juce::Colour::fromHSV(0.13f, 0.5f, 0.6f, 1.0f);
	juce::Colour medium = juce::Colour::fromHSV(0.13f, 0.5f, 0.5f, 1.0f);
	juce::Colour dark   = juce::Colour::fromHSV(0.13f, 0.5f, 0.4f, 1.0f);

	const int fonthHeight = (int)(SLIDER_WIDTH / FONT_DIVISOR);

	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];

		//Lable
		label.setText(StereoEnhancerAudioProcessor::paramsNames[i], juce::dontSendNotification);
		label.setFont(juce::Font(fonthHeight, juce::Font::bold));
		label.setJustificationType(juce::Justification::centred);
		addAndMakeVisible(label);

		//Slider
		slider.setLookAndFeel(&otherLookAndFeel);
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, SLIDER_FONT_SIZE);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, StereoEnhancerAudioProcessor::paramsNames[i], slider));
	}

	// Buttons
	addAndMakeVisible(monoButton);
	monoButton.setClickingTogglesState(true);
	buttonMonoAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonMono", monoButton));
	monoButton.setColour(juce::TextButton::buttonColourId, light);
	monoButton.setColour(juce::TextButton::buttonOnColourId, dark);
	monoButton.setLookAndFeel(&otherLookAndFeel);

	// Canvas
	setResizable(true, true);
	const float width = SLIDER_WIDTH * N_SLIDERS;
	setSize(width, SLIDER_WIDTH);

	if (auto* constrainer = getConstrainer())
	{
		constrainer->setFixedAspectRatio(N_SLIDERS);
		constrainer->setSizeLimits(width * 0.7f, SLIDER_WIDTH * 0.7, width * 2.0f, SLIDER_WIDTH * 2.0f);
	}
}

StereoEnhancerAudioProcessorEditor::~StereoEnhancerAudioProcessorEditor()
{
}

//==============================================================================
void StereoEnhancerAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(juce::Colour::fromHSV(0.13f, 0.5f, 0.7f, 1.0f));
}

void StereoEnhancerAudioProcessorEditor::resized()
{
	const int width = (int)(getWidth() / N_SLIDERS);
	const int height = getHeight();
	const int fonthHeight = (int)(height / FONT_DIVISOR);
	const int labelOffset = (int)(SLIDER_WIDTH / FONT_DIVISOR) + 5;

	// Sliders + Labels
	for (int i = 0; i < N_SLIDERS; ++i)
	{
		juce::Rectangle<int> rectangle;

		rectangle.setSize(width, height);
		rectangle.setPosition(i * width, 0);
		m_sliders[i].setBounds(rectangle);

		rectangle.removeFromBottom(labelOffset);
		m_labels[i].setBounds(rectangle);

		m_labels[i].setFont(juce::Font(fonthHeight, juce::Font::bold));
	}

	// Buttons
	const int posY = height - (int)(1.8f * fonthHeight);

	monoButton.setBounds((int)(getWidth() * (4.f / 5.0f) - 0.5f * fonthHeight), posY, fonthHeight, fonthHeight);
}