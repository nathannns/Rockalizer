#include "EchoModule.h"

void EchoModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    delayBuffer.setSize (static_cast<int> (spec.numChannels),
                         static_cast<int> (sampleRate * 4.0) + 4);
    toneState.assign (static_cast<size_t> (spec.numChannels), 0.0f);
    delaySamples.reset (sampleRate, 0.05);
    wetMix.reset (sampleRate, 0.02);
    feedbackValue.reset (sampleRate, 0.03);
    toneValue.reset (sampleRate, 0.03);
    wobbleValue.reset (sampleRate, 0.03);
    driveValue.reset (sampleRate, 0.03);
    reset();
}

void EchoModule::reset()
{
    delayBuffer.clear();
    std::fill (toneState.begin(), toneState.end(), 0.0f);
    writeIndex = 0;
    lfoPhase = 0.0f;
    delaySamples.setCurrentAndTargetValue (static_cast<float> (sampleRate * 0.375));
    wetMix.setCurrentAndTargetValue (0.0f);
    feedbackValue.setCurrentAndTargetValue (0.35f);
    toneValue.setCurrentAndTargetValue (7000.0f);
    wobbleValue.setCurrentAndTargetValue (0.0f);
    driveValue.setCurrentAndTargetValue (0.0f);
}

void EchoModule::setParameters (float timeMs, float repeats, float toneHz, float wobble,
                                float drive, float mix, bool enabled, int patternIndex)
{
    delaySamples.setTargetValue (juce::jlimit (0.04f, 2.5f, timeMs * 0.001f)
                                      * static_cast<float> (sampleRate));
    feedbackValue.setTargetValue (juce::jlimit (0.0f, 0.92f, repeats * 0.0092f));
    toneValue.setTargetValue (juce::jlimit (1200.0f, 14000.0f, toneHz));
    wobbleValue.setTargetValue (juce::jlimit (0.0f, 1.0f, wobble * 0.01f));
    driveValue.setTargetValue (juce::jlimit (0.0f, 1.0f, drive * 0.01f));
    wetMix.setTargetValue (enabled ? juce::jlimit (0.0f, 1.0f, mix * 0.01f) : 0.0f);
    pattern = juce::jlimit (0, 4, patternIndex);
}

float EchoModule::readDelay (int channel, float distance) const
{
    const auto size = delayBuffer.getNumSamples();
    auto position = static_cast<float> (writeIndex) - distance;
    while (position < 0.0f) position += static_cast<float> (size);
    while (position >= static_cast<float> (size)) position -= static_cast<float> (size);
    const auto first = static_cast<int> (position);
    const auto second = (first + 1) % size;
    const auto fraction = position - static_cast<float> (first);
    return juce::jmap (fraction, delayBuffer.getSample (channel, first),
                      delayBuffer.getSample (channel, second));
}

float EchoModule::processFeedbackTone (int channel, float sample, float cutoffHz)
{
    const auto coefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                               * cutoffHz / static_cast<float> (sampleRate));
    auto& state = toneState[static_cast<size_t> (channel)];
    state += coefficient * (sample - state);
    return state;
}

void EchoModule::getPattern (float* ratios, float* gains, int& taps) const
{
    taps = 1; ratios[0] = 1.0f; gains[0] = 1.0f;
    if (pattern == bounce)  { taps = 2; ratios[0] = 0.5f; ratios[1] = 1.0f; gains[0] = 0.65f; gains[1] = 0.85f; }
    if (pattern == gallop)  { taps = 2; ratios[0] = 0.67f; ratios[1] = 1.0f; gains[0] = 0.8f; gains[1] = 1.0f; }
    if (pattern == cluster) { taps = 3; ratios[0] = 0.5f; ratios[1] = 0.75f; ratios[2] = 1.0f; gains[0] = 0.45f; gains[1] = 0.6f; gains[2] = 0.8f; }
    if (pattern == wash)    { taps = 3; ratios[0] = 0.38f; ratios[1] = 0.63f; ratios[2] = 1.0f; gains[0] = 0.4f; gains[1] = 0.55f; gains[2] = 0.75f; }
}

void EchoModule::process (juce::AudioBuffer<float>& buffer)
{
    float ratios[3] {}, gains[3] {}; int taps = 1;
    getPattern (ratios, gains, taps);
    const auto channels = juce::jmin (buffer.getNumChannels(), delayBuffer.getNumChannels());
    const auto lfoStep = juce::MathConstants<float>::twoPi * 0.55f / static_cast<float> (sampleRate);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto baseDelay = delaySamples.getNextValue();
        const auto mix = wetMix.getNextValue();
        const auto feedback = feedbackValue.getNextValue();
        const auto toneCutoff = toneValue.getNextValue();
        const auto wobbleDepth = wobbleValue.getNextValue();
        const auto driveAmount = driveValue.getNextValue();

        for (int channel = 0; channel < channels; ++channel)
        {
            const auto phaseOffset = channel == 0 ? 0.0f : 1.7f;
            const auto modulation = std::sin (lfoPhase + phaseOffset)
                                  * wobbleDepth * static_cast<float> (sampleRate) * 0.004f;
            float wet = 0.0f;
            for (int tap = 0; tap < taps; ++tap)
                wet += readDelay (channel, juce::jmax (1.0f, baseDelay * ratios[tap] + modulation)) * gains[tap];
            wet /= std::sqrt (static_cast<float> (taps));

            auto feedbackSample = processFeedbackTone (channel, wet, toneCutoff);
            const auto gain = 1.0f + driveAmount * 5.0f;
            feedbackSample = std::tanh (feedbackSample * gain) / std::tanh (gain);

            const auto input = buffer.getSample (channel, sample);
            delayBuffer.setSample (channel, writeIndex,
                                   juce::jlimit (-2.0f, 2.0f, input + feedbackSample * feedback));
            buffer.setSample (channel, sample, input * (1.0f - mix) + wet * mix);
        }

        writeIndex = (writeIndex + 1) % delayBuffer.getNumSamples();
        lfoPhase += lfoStep;
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;
    }
}
