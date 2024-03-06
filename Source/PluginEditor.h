/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

class OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
	OtherLookAndFeel()
	{
		setColour(juce::Slider::thumbColourId, juce::Colours::red);
	}

	juce::Colour light = juce::Colour::fromHSV(0.13f, 0.5f, 0.6f, 1.0f);
	juce::Colour medium = juce::Colour::fromHSV(0.13f, 0.5f, 0.5f, 1.0f);
	juce::Colour dark = juce::Colour::fromHSV(0.13f, 0.5f, 0.4f, 1.0f);

	static const int SCALE = 70;
	static const int FONT_SIZE = 24;

	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
	{
		auto radius = ((float)juce::jmin(width / 2, height / 2) - 4.0f) * 0.9f;
		auto centreX = (float)x + (float)width  * 0.5f;
		auto centreY = (float)y + (float)height * 0.5f;
		auto rx = centreX - radius;
		auto ry = centreY - radius;
		auto rw = radius * 2.0f;
		auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

		// outline
		const float lineThickness = 6.0f;
		
		g.setColour(medium);
		g.drawEllipse(rx, ry, rw, rw, lineThickness);

		juce::Path p;
		auto pointerLength = radius * 0.2f;
		auto pointerThickness = lineThickness;
		p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
		p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

		// pointer
		g.setColour(medium);
		g.fillPath(p);
	}

	juce::Label *createSliderTextBox(juce::Slider &) override
	{
		auto *l = new juce::Label();
		l->setJustificationType(juce::Justification::centred);
		l->setFont(juce::Font(0.9f * FONT_SIZE * 0.01f * SCALE));
		return l;
	}

	void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool, bool isButtonDown) override
	{
		auto buttonArea = button.getLocalBounds();

		g.setColour(backgroundColour);
		g.fillRect(buttonArea);
	}
};

//==============================================================================
class StereoEnhancerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    StereoEnhancerAudioProcessorEditor (StereoEnhancerAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~StereoEnhancerAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 5;
	static const int SLIDER_WIDTH = 140;
	static const int SLIDER_FONT_SIZE = 20;

	static const int FONT_DIVISOR = 9;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StereoEnhancerAudioProcessor& audioProcessor;

	OtherLookAndFeel otherLookAndFeel;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

	juce::Label automationTLabel;
	juce::Label smoothingTypeLabel;
	juce::Label detectionTypeLabel;

	juce::TextButton monoButton{ "M" };

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonMonoAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoEnhancerAudioProcessorEditor)
};
