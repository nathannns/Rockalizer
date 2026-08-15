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
    RockalizerAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RockalizerAudioProcessorEditor)
};
