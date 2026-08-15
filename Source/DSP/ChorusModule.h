#pragma once

#include <JuceHeader.h>

class ChorusModule
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    void setParameters (float rateHz,
                        float depthPercent,
                        float widthPercent,
                        float toneHz,
                        float mixPercent,
                        bool enabled);

    void process (juce::AudioBuffer<float>& buffer);

private:
    juce::dsp::Chorus<float> chorus;
    juce::dsp::StateVariableTPTFilter<float> wetToneFilter;
    juce::AudioBuffer<float> wetBuffer;
    juce::SmoothedValue<float> wetMix;

    float stereoWidth = 1.0f;
    int maximumBlockSize = 0;
    int channelCount = 0;
};
