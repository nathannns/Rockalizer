#include "ChorusModule.h"

void ChorusModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    maximumBlockSize = static_cast<int> (spec.maximumBlockSize);
    channelCount = static_cast<int> (spec.numChannels);
    wetBuffer.setSize (channelCount, maximumBlockSize);

    chorus.prepare (spec);
    wetToneFilter.prepare (spec);
    wetToneFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    wetMix.reset (spec.sampleRate, 0.02);
    reset();
}

void ChorusModule::reset()
{
    chorus.reset();
    wetToneFilter.reset();
    wetMix.setCurrentAndTargetValue (0.0f);
}

void ChorusModule::setParameters (float rateHz,
                                  float depthPercent,
                                  float widthPercent,
                                  float toneHz,
                                  float mixPercent,
                                  bool enabled)
{
    chorus.setRate (juce::jlimit (0.05f, 5.0f, rateHz));
    chorus.setDepth (juce::jlimit (0.0f, 1.0f, depthPercent * 0.01f));
    chorus.setCentreDelay (7.0f);
    chorus.setFeedback (0.0f);
    chorus.setMix (1.0f);

    stereoWidth = juce::jlimit (0.0f, 2.0f, widthPercent * 0.02f);
    wetToneFilter.setCutoffFrequency (juce::jlimit (1000.0f, 16000.0f, toneHz));
    wetMix.setTargetValue (enabled ? juce::jlimit (0.0f, 1.0f, mixPercent * 0.01f) : 0.0f);
}

void ChorusModule::process (juce::AudioBuffer<float>& buffer)
{
    const auto samples = buffer.getNumSamples();
    const auto channels = juce::jmin (buffer.getNumChannels(), channelCount);

    jassert (samples <= maximumBlockSize);
    if (samples > maximumBlockSize || channels == 0)
        return;

    for (int channel = 0; channel < channels; ++channel)
        wetBuffer.copyFrom (channel, 0, buffer, channel, 0, samples);

    juce::dsp::AudioBlock<float> wetBlock (wetBuffer);
    auto activeWetBlock = wetBlock.getSubBlock (0, static_cast<size_t> (samples));
    juce::dsp::ProcessContextReplacing<float> wetContext (activeWetBlock);
    chorus.process (wetContext);
    wetToneFilter.process (wetContext);

    if (channels >= 2)
    {
        auto* left = wetBuffer.getWritePointer (0);
        auto* right = wetBuffer.getWritePointer (1);

        for (int sample = 0; sample < samples; ++sample)
        {
            const auto mid = 0.5f * (left[sample] + right[sample]);
            const auto side = 0.5f * (left[sample] - right[sample]) * stereoWidth;
            left[sample] = mid + side;
            right[sample] = mid - side;
        }
    }

    for (int sample = 0; sample < samples; ++sample)
    {
        const auto mix = wetMix.getNextValue();
        const auto dry = 1.0f - mix;

        for (int channel = 0; channel < channels; ++channel)
        {
            const auto wet = wetBuffer.getSample (channel, sample);
            const auto original = buffer.getSample (channel, sample);
            buffer.setSample (channel, sample, original * dry + wet * mix);
        }
    }
}
