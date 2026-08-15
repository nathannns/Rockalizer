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

    void configureKnob (juce::Slider& slider, const juce::String& suffix);

    RockalizerAudioProcessor& processor;

    juce::Slider inputSlider;
    juce::Slider lowCutSlider;
    juce::Slider highCutSlider;
    juce::Slider outputSlider;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> lowCutAttachment;
    std::unique_ptr<SliderAttachment> highCutAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RockalizerAudioProcessorEditor)
};
