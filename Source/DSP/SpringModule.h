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

    // A short direct head preserves guitar-monitoring latency while the long
    // IR tail uses larger, more CPU-efficient partitions.
    juce::dsp::Convolution convolution { juce::dsp::Convolution::NonUniform { 256 } };
    juce::dsp::StateVariableTPTFilter<float> toneFilter, bodyFilter, dripFilter;
    juce::AudioBuffer<float> wetBuffer, dripBuffer, dispersionBuffer, tailBuffer;
    juce::SmoothedValue<float> wetMix;
    std::atomic<int> requestedImpulse { 0 };
    int loadedImpulse = -1;
    int maximumBlockSize = 0, channelCount = 0;
    int currentModel = 0, cachedFilterModel = -1;
    int dispersionWriteIndex = 0;
    int tailWriteIndex = 0;
    float tailModPhase = 0.0f;
    double sampleRate = 44100.0;
    std::vector<float> inputEnvelope, dripEnvelope, dispersionDampingState, tailDampingState;
    // Real spring dispersion is a pure-phase (allpass) phenomenon -- higher
    // frequencies genuinely travel faster through the coiled wire, with no
    // magnitude change (Valimaki/Parker/Abel, "Parametric Spring
    // Reverberation Effect", JAES 2010). The existing dispersionBuffer taps
    // above already provide the large-scale (several-ms) delay spread across
    // 2-3 springs that gives the chirp its size; this cascade adds the
    // textbook-correct allpass mechanism as fine dispersion within each of
    // those taps, rather than the pure damped-delay coloring they had alone.
    static constexpr int numAllpassStages = 64;
    std::array<std::array<float, numAllpassStages>, 3> dispersionAllpassState {};
    float decayAmount = 0.5f, dwellAmount = 0.2f, dripAmount = 0.2f, toneAmount = 0.6f;
    float cachedToneHz = -1.0f;
    float dripDetectorCoefficient = 0.0f;
    float envelopeAttack = 0.0f, envelopeRelease = 0.0f;
};
