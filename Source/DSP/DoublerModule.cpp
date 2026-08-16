#include "DoublerModule.h"

void DoublerModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    delayBuffer.setSize (2, static_cast<int> (sampleRate * 0.04) + 4);
    delayBuffer.clear();
    amount.reset (sampleRate, 0.025);
    reset();
}

void DoublerModule::reset()
{
    amount.setCurrentAndTargetValue (0.0f);
    writeIndex = 0;
    validSamples = 0;
    driftPhase = 0.0f;
}

void DoublerModule::setAmount (float amountPercent)
{
    amount.setTargetValue (juce::jlimit (0.0f, 1.0f, amountPercent * 0.01f));
}

float DoublerModule::readDelay (int channel, float distance) const
{
    if (distance > static_cast<float> (validSamples))
        return 0.0f;
    const auto size = delayBuffer.getNumSamples();
    auto position = static_cast<float> (writeIndex) - distance;
    while (position < 0.0f) position += static_cast<float> (size);
    const auto first = static_cast<int> (position) % size;
    const auto second = (first + 1) % size;
    const auto fraction = position - std::floor (position);
    return juce::jmap (fraction, delayBuffer.getSample (channel, first),
                      delayBuffer.getSample (channel, second));
}

void DoublerModule::process (juce::AudioBuffer<float>& buffer)
{
    if (! amount.isSmoothing() && amount.getCurrentValue() <= 0.00001f)
        return;

    const auto channels = juce::jmin (2, buffer.getNumChannels());
    if (channels == 0)
        return;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto left = buffer.getSample (0, sample);
        const auto right = channels > 1 ? buffer.getSample (1, sample) : left;
        const auto mono = (left + right) * 0.5f;
        delayBuffer.setSample (0, writeIndex, mono);
        delayBuffer.setSample (1, writeIndex, mono);
        const auto control = amount.getNextValue();
        // One-knob interpretation of iZotope's workflow: Amount raises the
        // added voices, Separation moves them outward, and Variation adds
        // independent timing/pitch differences. The original stereo signal
        // remains intact and anchors pitch and attack.
        const auto variation = 0.05f + control * 0.30f;
        const auto leftDrift = (std::sin (driftPhase) * 0.72f
                              + std::sin (driftPhase * 0.37f + 1.1f) * 0.28f) * variation;
        const auto rightDrift = (std::sin (driftPhase * 0.83f + 2.2f) * 0.76f
                               + std::sin (driftPhase * 0.29f + 0.4f) * 0.24f) * variation;
        const auto leftDelayMs = 9.0f + control * 2.2f + leftDrift;
        const auto rightDelayMs = 13.0f + control * 3.8f + rightDrift;
        const auto delayedLeft = readDelay (0, leftDelayMs * 0.001f * static_cast<float> (sampleRate));
        const auto delayedRight = readDelay (1, rightDelayMs * 0.001f * static_cast<float> (sampleRate));
        const auto copyGain = control * 0.62f;
        const auto separation = 0.30f + control * 0.70f;
        const auto crossGain = copyGain * (1.0f - separation) * 0.35f;
        const auto directGain = 1.0f - control * 0.08f;

        buffer.setSample (0, sample, left * directGain
                                      + delayedLeft * copyGain + delayedRight * crossGain);
        if (channels > 1)
            buffer.setSample (1, sample, right * directGain
                                          + delayedRight * copyGain + delayedLeft * crossGain);

        writeIndex = (writeIndex + 1) % delayBuffer.getNumSamples();
        validSamples = juce::jmin (validSamples + 1, delayBuffer.getNumSamples());
        driftPhase += juce::MathConstants<float>::twoPi * 0.095f / static_cast<float> (sampleRate);
        if (driftPhase >= juce::MathConstants<float>::twoPi)
            driftPhase -= juce::MathConstants<float>::twoPi;
    }
}
