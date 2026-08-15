#pragma once
#include <JuceHeader.h>

class TapeModule
{
public:
    enum Type { studio, cassette };
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (float drive, float compression, float tone, float age,
                        float mix, bool enabled, int type);
    void process (juce::AudioBuffer<float>& buffer);

private:
    float readWow (int channel, float delayInSamples) const;
    juce::AudioBuffer<float> wowBuffer;
    juce::SmoothedValue<float> wetMix;
    std::vector<float> envelope, toneState, bassState;
    juce::Random noise { 0x524f434b };
    double sampleRate = 44100.0;
    int writeIndex = 0, tapeType = studio;
    float driveValue = 0.0f, compValue = 0.0f, toneValue = 0.5f, ageValue = 0.0f;
    float lfoPhase = 0.0f;
};
