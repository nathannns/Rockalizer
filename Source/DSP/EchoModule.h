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
    float processFeedbackTone (int channel, float sample);
    void getPattern (float* ratios, float* gains, int& taps) const;

    juce::AudioBuffer<float> delayBuffer;
    juce::SmoothedValue<float> delaySamples;
    juce::SmoothedValue<float> wetMix;
    std::vector<float> toneState;
    double sampleRate = 44100.0;
    int writeIndex = 0;
    int pattern = straight;
    float feedback = 0.35f;
    float toneCutoff = 7000.0f;
    float wobbleDepth = 0.0f;
    float driveAmount = 0.0f;
    float lfoPhase = 0.0f;
};
