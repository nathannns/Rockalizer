#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class RockalizerAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit RockalizerAudioProcessorEditor (RockalizerAudioProcessor&);
    ~RockalizerAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void configureKnob (juce::Slider& slider,
                        juce::Label& label,
                        const juce::String& name,
                        const juce::String& suffix);

    RockalizerAudioProcessor& processor;

    juce::Slider inputSlider;
    juce::Slider lowCutSlider;
    juce::Slider highCutSlider;
    juce::Slider outputSlider;
    juce::Slider chorusRateSlider;
    juce::Slider chorusDepthSlider;
    juce::Slider chorusWidthSlider;
    juce::Slider chorusToneSlider;
    juce::Slider chorusMixSlider;

    juce::Label chorusRateLabel;
    juce::Label chorusDepthLabel;
    juce::Label chorusWidthLabel;
    juce::Label chorusToneLabel;
    juce::Label chorusMixLabel;
    juce::ToggleButton chorusBypassButton { "ON" };

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> lowCutAttachment;
    std::unique_ptr<SliderAttachment> highCutAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> chorusRateAttachment;
    std::unique_ptr<SliderAttachment> chorusDepthAttachment;
    std::unique_ptr<SliderAttachment> chorusWidthAttachment;
    std::unique_ptr<SliderAttachment> chorusToneAttachment;
    std::unique_ptr<SliderAttachment> chorusMixAttachment;
    std::unique_ptr<ButtonAttachment> chorusBypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RockalizerAudioProcessorEditor)
};
