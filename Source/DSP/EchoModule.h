#pragma once

#include <JuceHeader.h>

class EchoModule
{
public:
    enum Pattern { straight, bounce, gallop, cluster, wash };

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (float timeMs, float repeats, float toneHz, float wobble,
                        float drive, float mix, bool enabled, int patternIndex);
    void process (juce::AudioBuffer<float>& buffer);

private:
    float readDelay (int channel, float delaySamples) const;
    float processFeedbackTone (int channel, float sample, float cutoffHz);
    void getPattern (float* ratios, float* gains, int& taps) const;

    juce::AudioBuffer<float> delayBuffer;
    juce::SmoothedValue<float> delaySamples;
    juce::SmoothedValue<float> wetMix;
    juce::SmoothedValue<float> feedbackValue;
    juce::SmoothedValue<float> toneValue;
    juce::SmoothedValue<float> wobbleValue;
    juce::SmoothedValue<float> driveValue;
    std::vector<float> toneState;
    double sampleRate = 44100.0;
    int writeIndex = 0;
    int pattern = straight;
    float lfoPhase = 0.0f;
};
