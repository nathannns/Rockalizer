#include "ChorusModule.h"

void ChorusModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    const auto channels = static_cast<int> (spec.numChannels);
    delayBuffer.setSize (channels, static_cast<int> (sampleRate * 0.04) + 4);
    toneState.assign (static_cast<size_t> (channels), 0.0f);
    crossLowState.assign (static_cast<size_t> (channels), 0.0f);
    feedbackState.assign (static_cast<size_t> (channels), 0.0f);

    for (auto* value : { &rateValue, &depthValue, &widthValue, &toneValue, &wetMix })
        value->reset (sampleRate, 0.03);
    reset();
}

void ChorusModule::reset()
{
    delayBuffer.clear();
    std::fill (toneState.begin(), toneState.end(), 0.0f);
    std::fill (crossLowState.begin(), crossLowState.end(), 0.0f);
    std::fill (feedbackState.begin(), feedbackState.end(), 0.0f);
    writeIndex = 0;
    lfoPhase = 0.0f;
    rateValue.setCurrentAndTargetValue (0.32f);
    depthValue.setCurrentAndTargetValue (0.75f);
    widthValue.setCurrentAndTargetValue (0.75f);
    toneValue.setCurrentAndTargetValue (8000.0f);
    wetMix.setCurrentAndTargetValue (0.0f);
}

void ChorusModule::setParameters (float rateHz, float depthPercent, float widthPercent,
                                  float toneHz, float mixPercent, bool enabled)
{
    // Dimension-style range: slow, shallow dual modulation creates width and
    // depth without the obvious pitch sweep of a conventional chorus.
    const auto normalisedRate = juce::jlimit (0.0f, 1.0f, (rateHz - 0.05f) / 4.95f);
    rateValue.setTargetValue (0.10f + std::pow (normalisedRate, 0.70f) * 0.72f);
    depthValue.setTargetValue (std::pow (juce::jlimit (0.0f, 1.0f, depthPercent * 0.01f), 0.74f));
    widthValue.setTargetValue (juce::jlimit (0.0f, 1.0f, widthPercent * 0.01f));
    toneValue.setTargetValue (juce::jlimit (1800.0f, 16000.0f, toneHz));
    wetMix.setTargetValue (enabled ? juce::jlimit (0.0f, 1.0f, mixPercent * 0.01f) : 0.0f);
}

float ChorusModule::readDelay (int channel, float distance) const
{
    const auto size = delayBuffer.getNumSamples();
    auto position = static_cast<float> (writeIndex) - distance;
    while (position < 0.0f) position += static_cast<float> (size);
    while (position >= static_cast<float> (size)) position -= static_cast<float> (size);
    const auto first = static_cast<int> (position);
    const auto second = (first + 1) % size;
    return juce::jmap (position - static_cast<float> (first),
                       delayBuffer.getSample (channel, first),
                       delayBuffer.getSample (channel, second));
}

void ChorusModule::process (juce::AudioBuffer<float>& buffer)
{
    if (! wetMix.isSmoothing() && wetMix.getCurrentValue() <= 0.00001f)
        return;

    const auto channels = juce::jmin (2, buffer.getNumChannels(), delayBuffer.getNumChannels());
    if (channels == 0)
        return;

    const auto baseDelayA = static_cast<float> (sampleRate) * 0.0068f;
    const auto baseDelayB = static_cast<float> (sampleRate) * 0.0104f;
    const auto crossLowCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                       * 180.0f / static_cast<float> (sampleRate));

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float original[2] {};
        float wet[2] {};
        for (int channel = 0; channel < channels; ++channel)
        {
            original[channel] = buffer.getSample (channel, sample);
            const auto other = channels >= 2 ? 1 - channel : channel;
            const auto crossFeedback = feedbackState[static_cast<size_t> (other)] * 0.035f;
            delayBuffer.setSample (channel, writeIndex, original[channel] + crossFeedback);
        }

        const auto rate = rateValue.getNextValue();
        const auto depth = depthValue.getNextValue();
        const auto width = widthValue.getNextValue();
        const auto cutoff = toneValue.getNextValue();
        const auto mix = wetMix.getNextValue();
        const auto depthSamples = static_cast<float> (sampleRate)
                                * (0.00010f + depth * 0.00110f);
        const auto toneCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                       * cutoff / static_cast<float> (sampleRate));

        for (int channel = 0; channel < channels; ++channel)
        {
            const auto stereoPhase = channel == 0 ? 0.0f : juce::MathConstants<float>::pi;
            const auto triangleA = (2.0f / juce::MathConstants<float>::pi)
                                 * std::asin (std::sin (lfoPhase + stereoPhase));
            const auto triangleB = (2.0f / juce::MathConstants<float>::pi)
                                 * std::asin (std::sin (lfoPhase + stereoPhase
                                                       + juce::MathConstants<float>::halfPi));
            const auto tapA = readDelay (channel, baseDelayA + triangleA * depthSamples);
            const auto tapB = readDelay (channel, baseDelayB - triangleB * depthSamples * 0.62f);
            auto dimensionWet = tapA * 0.56f + tapB * 0.44f;
            auto& state = toneState[static_cast<size_t> (channel)];
            state += toneCoefficient * (dimensionWet - state);
            wet[channel] = state;
            feedbackState[static_cast<size_t> (channel)] = state;
        }

        if (channels >= 2)
        {
            const auto rawLeft = wet[0];
            const auto rawRight = wet[1];
            crossLowState[0] += crossLowCoefficient * (rawRight - crossLowState[0]);
            crossLowState[1] += crossLowCoefficient * (rawLeft - crossLowState[1]);
            const auto crossAmount = 0.08f + width * 0.26f;
            wet[0] = rawLeft - (rawRight - crossLowState[0]) * crossAmount;
            wet[1] = rawRight - (rawLeft - crossLowState[1]) * crossAmount;
        }

        for (int channel = 0; channel < channels; ++channel)
            buffer.setSample (channel, sample, original[channel] * (1.0f - mix * 0.12f)
                                                + wet[channel] * mix * 0.72f);

        writeIndex = (writeIndex + 1) % delayBuffer.getNumSamples();
        lfoPhase += juce::MathConstants<float>::twoPi * rate / static_cast<float> (sampleRate);
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;
    }
}
