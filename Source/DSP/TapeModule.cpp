#include "TapeModule.h"

void TapeModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    const auto channels = static_cast<int> (spec.numChannels);
    wowBuffer.setSize (channels, static_cast<int> (sampleRate * 0.04) + 4);
    envelope.assign (channels, 0.0f); toneState.assign (channels, 0.0f); bassState.assign (channels, 0.0f);
    wetMix.reset (sampleRate, 0.02);
    reset();
}

void TapeModule::reset()
{
    wowBuffer.clear(); writeIndex = 0; lfoPhase = 0.0f;
    std::fill (envelope.begin(), envelope.end(), 0.0f);
    std::fill (toneState.begin(), toneState.end(), 0.0f);
    std::fill (bassState.begin(), bassState.end(), 0.0f);
    wetMix.setCurrentAndTargetValue (0.0f);
}

void TapeModule::setParameters (float drive, float compression, float tone, float age,
                                float mix, bool enabled, int type)
{
    driveValue = juce::jlimit (0.0f, 1.0f, drive * 0.01f);
    compValue = juce::jlimit (0.0f, 1.0f, compression * 0.01f);
    toneValue = juce::jlimit (0.0f, 1.0f, tone * 0.01f);
    ageValue = juce::jlimit (0.0f, 1.0f, age * 0.01f);
    tapeType = juce::jlimit (0, 1, type);
    wetMix.setTargetValue (enabled ? juce::jlimit (0.0f, 1.0f, mix * 0.01f) : 0.0f);
}

float TapeModule::readWow (int channel, float distance) const
{
    const auto size = wowBuffer.getNumSamples();
    auto position = static_cast<float> (writeIndex) - distance;
    while (position < 0.0f) position += static_cast<float> (size);
    const auto first = static_cast<int> (position) % size;
    const auto second = (first + 1) % size;
    return juce::jmap (position - std::floor (position), wowBuffer.getSample (channel, first),
                      wowBuffer.getSample (channel, second));
}

void TapeModule::process (juce::AudioBuffer<float>& buffer)
{
    const auto channels = juce::jmin (buffer.getNumChannels(), wowBuffer.getNumChannels());
    const auto cassetteMode = tapeType == cassette;
    const auto inputDb = driveValue * (cassetteMode ? 24.0f : 18.0f);
    const auto inputGain = juce::Decibels::decibelsToGain (inputDb);
    const auto threshold = cassetteMode ? 0.28f : 0.48f;
    const auto ratio = 1.0f + compValue * (cassetteMode ? 7.0f : 3.5f);
    const auto cutoffTop = cassetteMode ? 13500.0f : 20000.0f;
    const auto cutoffBottom = cassetteMode ? 2500.0f : 5000.0f;
    const auto cutoff = juce::jmap (toneValue, cutoffBottom, cutoffTop)
                      * (1.0f - ageValue * (cassetteMode ? 0.55f : 0.25f));
    const auto toneCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                   * cutoff / static_cast<float> (sampleRate));
    const auto bassCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                   * (cassetteMode ? 130.0f : 85.0f)
                                                   / static_cast<float> (sampleRate));
    const auto attack = std::exp (-1.0f / (static_cast<float> (sampleRate) * 0.008f));
    const auto release = std::exp (-1.0f / (static_cast<float> (sampleRate) * (cassetteMode ? 0.16f : 0.28f)));
    const auto wowRate = cassetteMode ? 0.75f : 0.32f;
    const auto wowDepthMs = ageValue * (cassetteMode ? 3.2f : 0.45f);
    const auto baseDelay = static_cast<float> (sampleRate) * 0.008f;
    const auto hissGain = juce::Decibels::decibelsToGain ((cassetteMode ? -62.0f : -88.0f)
                                                          + ageValue * (cassetteMode ? 15.0f : 12.0f));

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto mix = wetMix.getNextValue();
        for (int channel = 0; channel < channels; ++channel)
        {
            const auto dry = buffer.getSample (channel, sample);
            wowBuffer.setSample (channel, writeIndex, dry);
            const auto offset = channel == 0 ? 0.0f : 1.3f;
            const auto wow = std::sin (lfoPhase + offset) * wowDepthMs * 0.001f
                           * static_cast<float> (sampleRate);
            auto x = readWow (channel, baseDelay + wow) * inputGain;

            const auto level = std::abs (x);
            auto& env = envelope[static_cast<size_t> (channel)];
            env = level > env ? attack * env + (1.0f - attack) * level
                              : release * env + (1.0f - release) * level;
            if (env > threshold)
                x *= std::pow (env / threshold, -(1.0f - 1.0f / ratio));

            const auto bias = cassetteMode ? 0.045f : 0.018f;
            const auto shape = cassetteMode ? 1.75f : 1.25f;
            x = (std::tanh (x * shape + bias) - std::tanh (bias)) / std::tanh (shape);

            auto& lp = toneState[static_cast<size_t> (channel)];
            lp += toneCoefficient * (x - lp);
            auto& bass = bassState[static_cast<size_t> (channel)];
            bass += bassCoefficient * (lp - bass);
            x = lp + bass * (cassetteMode ? 0.035f : 0.085f);
            x += (noise.nextFloat() * 2.0f - 1.0f) * hissGain * ageValue;
            x *= juce::Decibels::decibelsToGain (-inputDb * (cassetteMode ? 0.34f : 0.25f));
            buffer.setSample (channel, sample, dry * (1.0f - mix) + x * mix);
        }
        writeIndex = (writeIndex + 1) % wowBuffer.getNumSamples();
        lfoPhase += juce::MathConstants<float>::twoPi * wowRate / static_cast<float> (sampleRate);
        if (lfoPhase >= juce::MathConstants<float>::twoPi) lfoPhase -= juce::MathConstants<float>::twoPi;
    }
}
