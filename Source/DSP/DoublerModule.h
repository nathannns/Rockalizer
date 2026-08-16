#pragma once

#include <JuceHeader.h>

class DoublerModule
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setAmount (float amountPercent);
    void process (juce::AudioBuffer<float>& buffer);

private:
    float readDelay (int channel, float distance) const;

    juce::AudioBuffer<float> delayBuffer;
    juce::SmoothedValue<float> amount;
    double sampleRate = 44100.0;
    int writeIndex = 0;
    int validSamples = 0;
    float driftPhase = 0.0f;
};
