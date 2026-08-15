#pragma once

#include <JuceHeader.h>

class ChorusModule
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (float rateHz, float depthPercent, float widthPercent,
                        float toneHz, float mixPercent, bool enabled);
    void process (juce::AudioBuffer<float>& buffer);

private:
    float readDelay (int channel, float distance) const;

    juce::AudioBuffer<float> delayBuffer;
    juce::SmoothedValue<float> rateValue, depthValue, widthValue, toneValue, wetMix;
    std::vector<float> toneState, crossLowState, feedbackState;
    double sampleRate = 44100.0;
    int writeIndex = 0;
    float lfoPhase = 0.0f;
};
