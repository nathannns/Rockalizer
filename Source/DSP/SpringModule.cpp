#include "SpringModule.h"

void SpringModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    maximumBlockSize = static_cast<int> (spec.maximumBlockSize);
    channelCount = static_cast<int> (spec.numChannels);
    wetBuffer.setSize (channelCount, maximumBlockSize);
    dripBuffer.setSize (channelCount, maximumBlockSize);
    convolution.prepare (spec); toneFilter.prepare (spec); dripFilter.prepare (spec);
    toneFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    dripFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    dripFilter.setCutoffFrequency (2600.0f); dripFilter.setResonance (0.82f);
    wetMix.reset (spec.sampleRate, 0.03);
    reset();
    triggerAsyncUpdate();
}

void SpringModule::reset()
{
    convolution.reset(); toneFilter.reset(); dripFilter.reset();
    wetMix.setCurrentAndTargetValue (0.0f);
}

void SpringModule::setParameters (float decay, float dwell, float tone, float drip, float mix,
                                  bool enabled, int impulseIndex)
{
    decayAmount = juce::jlimit (0.0f, 1.0f, decay * 0.01f);
    dwellAmount = juce::jlimit (0.0f, 1.0f, dwell * 0.01f);
    dripAmount = juce::jlimit (0.0f, 1.0f, drip * 0.01f);
    toneFilter.setCutoffFrequency (juce::jmap (juce::jlimit (0.0f, 1.0f, tone * 0.01f),
                                               1800.0f, 15000.0f));
    wetMix.setTargetValue (enabled ? juce::jlimit (0.0f, 1.0f, mix * 0.01f) : 0.0f);
    impulseIndex = juce::jlimit (0, 6, impulseIndex);
    if (requestedImpulse.exchange (impulseIndex) != impulseIndex || loadedImpulse != impulseIndex)
        triggerAsyncUpdate();
}

void SpringModule::handleAsyncUpdate() { loadImpulse (requestedImpulse.load()); }

void SpringModule::loadImpulse (int index)
{
    const void* data[] { BinaryData::spring_gbsr_wav, BinaryData::spring_deluxe_wav,
        BinaryData::spring_space_wav, BinaryData::spring_9100_wav, BinaryData::spring_echomixer_wav,
        BinaryData::spring_schaller_wav, BinaryData::spring_pioneer_wav };
    const int sizes[] { BinaryData::spring_gbsr_wavSize, BinaryData::spring_deluxe_wavSize,
        BinaryData::spring_space_wavSize, BinaryData::spring_9100_wavSize, BinaryData::spring_echomixer_wavSize,
        BinaryData::spring_schaller_wavSize, BinaryData::spring_pioneer_wavSize };
    convolution.loadImpulseResponse (data[index], static_cast<size_t> (sizes[index]),
        juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, 0,
        juce::dsp::Convolution::Normalise::yes);
    loadedImpulse = index;
}

void SpringModule::process (juce::AudioBuffer<float>& buffer)
{
    const auto samples = buffer.getNumSamples();
    const auto channels = juce::jmin (buffer.getNumChannels(), channelCount);
    if (samples > maximumBlockSize || channels == 0) return;
    for (int channel = 0; channel < channels; ++channel)
        wetBuffer.copyFrom (channel, 0, buffer, channel, 0, samples);

    const auto dwellGain = 1.0f + dwellAmount * 5.0f;
    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
            wetBuffer.setSample (channel, sample,
                std::tanh (wetBuffer.getSample (channel, sample) * dwellGain) / std::tanh (dwellGain));

    juce::dsp::AudioBlock<float> wetBlock (wetBuffer);
    auto activeWet = wetBlock.getSubBlock (0, static_cast<size_t> (samples));
    juce::dsp::ProcessContextReplacing<float> wetContext (activeWet);
    convolution.process (wetContext);
    for (int channel = 0; channel < channels; ++channel)
        dripBuffer.copyFrom (channel, 0, wetBuffer, channel, 0, samples);
    juce::dsp::AudioBlock<float> dripBlock (dripBuffer);
    auto activeDrip = dripBlock.getSubBlock (0, static_cast<size_t> (samples));
    juce::dsp::ProcessContextReplacing<float> dripContext (activeDrip);
    dripFilter.process (dripContext); toneFilter.process (wetContext);

    const auto tailGain = juce::jmap (decayAmount, 0.65f, 1.35f);
    for (int sample = 0; sample < samples; ++sample)
    {
        const auto mix = wetMix.getNextValue();
        for (int channel = 0; channel < channels; ++channel)
        {
            const auto wet = (wetBuffer.getSample (channel, sample)
                            + dripBuffer.getSample (channel, sample) * dripAmount * 0.8f) * tailGain;
            const auto dry = buffer.getSample (channel, sample);
            buffer.setSample (channel, sample, dry * (1.0f - mix) + wet * mix);
        }
    }
}
