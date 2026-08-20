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
    bool isWetTransitionActive() const noexcept
    {
        return wetMix.isSmoothing() || wetMix.getCurrentValue() > 0.00001f;
    }
    // Host-side latency (in samples) introduced by the oversampler, so the
    // processor can report it via getLatencySamples() and the host can PDC-
    // align the wet path against the dry/wet-mix path. Oversampling is the
    // only latency-bearing stage in this module (the delay line itself is
    // read/write-aligned, not delayed), so this is zero when oversampling
    // is off.
    int getLatencySamples() const noexcept
    {
        if (oversamplingChoice == 1 && oversampling2x != nullptr)
            return juce::roundToInt (oversampling2x->getLatencyInSamples());
        if (oversamplingChoice == 2 && oversampling4x != nullptr)
            return juce::roundToInt (oversampling4x->getLatencyInSamples());
        return 0;
    }

private:
    void processCore (juce::AudioBuffer<float>& buffer, double processingRate);
    float readWow (int channel, float delayInSamples) const;
    juce::AudioBuffer<float> wowBuffer;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling2x, oversampling4x;
    juce::SmoothedValue<float> wetMix;
    std::vector<float> envelope, detectorLowState, magnetisationState;
    std::vector<float> toneState, bassState, midState;
    // Previous sample's driven (post-record-gain) value per channel, so the
    // saturation curve can tell whether the field is currently rising or
    // falling -- the actual defining signature of magnetic hysteresis.
    std::vector<float> previousDrivenState;
    // Lowpassed version of the raw rising/falling sign above (see
    // processCore's directionSmooth comment) -- a real tape's hysteresis
    // loop doesn't flip its direction state on every sample-to-sample
    // wiggle, so smoothing the instantaneous +-1 sign rather than using it
    // directly avoids injecting broadband, alias-prone "fizz" on harmonic-
    // rich, heavily-driven signal.
    std::vector<float> directionSmoothState;
    double sampleRate = 44100.0;
    int writeIndex = 0, tapeType = studio, oversamplingChoice = 0;
    int validSamples = 0;
    float driveValue = 0.0f, compValue = 0.0f, toneValue = 0.5f, ageValue = 0.0f;
    float lfoPhase = 0.0f;
};
