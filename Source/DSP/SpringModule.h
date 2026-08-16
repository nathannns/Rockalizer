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
    void loadImpulse (int index, int decayStep);

    // A short direct head preserves guitar-monitoring latency while the long
    // IR tail uses larger, more CPU-efficient partitions.
    juce::dsp::Convolution convolution { juce::dsp::Convolution::NonUniform { 256 } };
    juce::dsp::StateVariableTPTFilter<float> toneFilter, bodyFilter, dripFilter;
    juce::AudioBuffer<float> wetBuffer, dripBuffer, tailBuffer;
    juce::SmoothedValue<float> wetMix;
    std::atomic<int> requestedImpulse { 0 };
    std::atomic<int> requestedDecayStep { 8 };
    int loadedImpulse = -1, loadedDecayStep = -1;
    int maximumBlockSize = 0, channelCount = 0;
    int tailWriteIndex = 0;
    double sampleRate = 44100.0;
    std::vector<float> inputEnvelope, dripEnvelope, tailDampingState;
    float decayAmount = 0.5f, dwellAmount = 0.2f, dripAmount = 0.2f;
    float cachedToneHz = -1.0f;
    float dripDetectorCoefficient = 0.0f;
    float envelopeAttack = 0.0f, envelopeRelease = 0.0f;
};
