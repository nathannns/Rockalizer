#pragma once
#include <JuceHeader.h>

class TapeModule
{
public:
    enum Type { studio, cassette };
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (float drive, float compression, float tone, float age,
                        float mix, bool enabled, int type, int oversamplingMode);
    void process (juce::AudioBuffer<float>& buffer);

private:
    void processCore (juce::AudioBuffer<float>& buffer, double processingRate);
    float readWow (int channel, float delayInSamples) const;
    juce::AudioBuffer<float> wowBuffer;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling2x, oversampling4x;
    juce::SmoothedValue<float> wetMix;
    std::vector<float> envelope, detectorLowState, magnetisationState;
    std::vector<float> toneState, bassState, midState;
    double sampleRate = 44100.0;
    int writeIndex = 0, tapeType = studio, oversamplingChoice = 0;
    int validSamples = 0;
    float driveValue = 0.0f, compValue = 0.0f, toneValue = 0.5f, ageValue = 0.0f;
    float lfoPhase = 0.0f;
};
