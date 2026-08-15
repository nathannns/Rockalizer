#pragma once
#include <JuceHeader.h>
#include <BinaryData.h>

class SpringModule : private juce::AsyncUpdater
{
public:
    ~SpringModule() override { cancelPendingUpdate(); }
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (float decay, float dwell, float tone, float drip, float mix,
                        bool enabled, int impulseIndex);
    void process (juce::AudioBuffer<float>& buffer);

private:
    void handleAsyncUpdate() override;
    void loadImpulse (int index);

    juce::dsp::Convolution convolution;
    juce::dsp::StateVariableTPTFilter<float> toneFilter, dripFilter;
    juce::AudioBuffer<float> wetBuffer, dripBuffer;
    juce::SmoothedValue<float> wetMix;
    std::atomic<int> requestedImpulse { 0 };
    int loadedImpulse = -1, maximumBlockSize = 0, channelCount = 0;
    float decayAmount = 0.5f, dwellAmount = 0.2f, dripAmount = 0.2f;
};
