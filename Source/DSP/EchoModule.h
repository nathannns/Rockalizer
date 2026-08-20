#pragma once

#include <JuceHeader.h>
#include "Antialiasing.h"

// Roland RE-201 Space Echo-inspired: 3 tape playback heads at fixed,
// equally-spaced positions (delay ratio exactly 1:2:3 off head 1's time).
// Pattern names stay the original friendly ones (Straight/Bounce/Gallop/
// Cluster/Wash) — see getPattern() in the .cpp for how each maps to a real
// head combination underneath. Ping-Pong is a new, explicitly modern bonus
// mode, not an RE-201 characteristic (the real unit is mono).
//
// Section-name mapping: this is Rockalizer's "delay" pedal, the SAME effect
// as Threadline's "Satellite - 201" pedal (Threadline's SpaceEchoModule.h).
// Threadline's SpaceEchoModule is the UPGRADED version of this: it swaps
// this single-"Tone" filter + plain tanh drive for a passive Bass/Treble
// shelving pair + a Jiles-Atherton hysteresis drive. Keeping the class/file
// named EchoModule here (rather than SpaceEchoModule) is a deliberate
// mirror of the original cross-repo split -- see Threadline's SpaceEchoModule
// header for the correspondence.
class EchoModule
{
public:
    enum Pattern { straight, bounce, gallop, cluster, wash, pingPong };

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setParameters (float timeMs, float repeats, float toneHz, float wobble,
                        float drive, float mix, bool enabled, int patternIndex);
    void process (juce::AudioBuffer<float>& buffer);
    bool isWetTransitionActive() const noexcept
    {
        return wetMix.isSmoothing() || wetMix.getCurrentValue() > 0.00001f;
    }

private:
    float readDelay (int channel, float delaySamples) const;
    float processFeedbackTone (int channel, float sample);
    void getPattern (float* ratios, float* gains, int& taps) const;

    juce::AudioBuffer<float> delayBuffer;
    juce::SmoothedValue<float> delaySamples;
    juce::SmoothedValue<float> wetMix;
    juce::SmoothedValue<float> feedbackValue;
    juce::SmoothedValue<float> wobbleValue;
    juce::SmoothedValue<float> driveValue;
    std::vector<float> toneState;
    // Antialiased (ADAA, see Antialiasing.h) versions of the feedback-loop
    // nonlinearities -- the drive saturation and the write-side safety
    // rail -- indexed 0/1 per delay line regardless of whether that line
    // is being driven by the main (per-channel) path or Ping-Pong's
    // explicit line-0/line-1 addressing.
    AdaaTanh feedbackDriveSaturation[2];
    AdaaSmoothRail writeRail[2];
    double sampleRate = 44100.0;
    int writeIndex = 0;
    int validSamples = 0;
    int pattern = straight;
    float lfoPhase = 0.0f;
    float flutterPhase = 0.0f;
    float toneCoefficient = 1.0f;
    float cachedToneHz = -1.0f;
};
