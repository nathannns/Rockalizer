#include "SpringModule.h"

void SpringModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maximumBlockSize = static_cast<int> (spec.maximumBlockSize);
    channelCount = static_cast<int> (spec.numChannels);
    wetBuffer.setSize (channelCount, maximumBlockSize);
    dripBuffer.setSize (channelCount, maximumBlockSize);
    convolution.prepare (spec); toneFilter.prepare (spec); bodyFilter.prepare (spec); dripFilter.prepare (spec);
    toneFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    bodyFilter.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    bodyFilter.setCutoffFrequency (115.0f);
    dripFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    dripFilter.setCutoffFrequency (2450.0f); dripFilter.setResonance (0.76f);
    inputEnvelope.assign (static_cast<size_t> (channelCount), 0.0f);
    wetMix.reset (spec.sampleRate, 0.03);
    reset();
    triggerAsyncUpdate();
}

void SpringModule::reset()
{
    convolution.reset(); toneFilter.reset(); bodyFilter.reset(); dripFilter.reset();
    std::fill (inputEnvelope.begin(), inputEnvelope.end(), 0.0f);
    wetMix.setCurrentAndTargetValue (0.0f);
}

void SpringModule::setParameters (float decay, float dwell, float tone, float drip, float mix,
                                  bool enabled, int impulseIndex)
{
    decayAmount = juce::jlimit (0.0f, 1.0f, decay * 0.01f);
    dwellAmount = juce::jlimit (0.0f, 1.0f, dwell * 0.01f);
    dripAmount = juce::jlimit (0.0f, 1.0f, drip * 0.01f);
    toneFilter.setCutoffFrequency (juce::jmap (juce::jlimit (0.0f, 1.0f, tone * 0.01f),
                                               1400.0f, 12500.0f));
    wetMix.setTargetValue (enabled ? juce::jlimit (0.0f, 1.0f, mix * 0.01f) : 0.0f);
    impulseIndex = juce::jlimit (0, 6, impulseIndex);
    const auto decayStep = juce::jlimit (0, 15, juce::roundToInt (decayAmount * 15.0f));
    const auto impulseChanged = requestedImpulse.exchange (impulseIndex) != impulseIndex;
    const auto decayChanged = requestedDecayStep.exchange (decayStep) != decayStep;
    if (impulseChanged || decayChanged || loadedImpulse != impulseIndex || loadedDecayStep != decayStep)
        triggerAsyncUpdate();
}

void SpringModule::handleAsyncUpdate()
{
    const auto impulse = requestedImpulse.load();
    const auto decayStep = requestedDecayStep.load();
    loadImpulse (impulse, decayStep);
    if (impulse != requestedImpulse.load() || decayStep != requestedDecayStep.load())
        triggerAsyncUpdate();
}

void SpringModule::loadImpulse (int index, int decayStep)
{
    const void* data[] { BinaryData::spring_gbsr_clean_wav, BinaryData::spring_deluxe_clean_wav,
        BinaryData::spring_space_clean_wav, BinaryData::spring_9100_clean_wav,
        BinaryData::spring_echomixer_clean_wav, BinaryData::spring_schaller_clean_wav,
        BinaryData::spring_pioneer_clean_wav };
    const int sizes[] { BinaryData::spring_gbsr_clean_wavSize, BinaryData::spring_deluxe_clean_wavSize,
        BinaryData::spring_space_clean_wavSize, BinaryData::spring_9100_clean_wavSize,
        BinaryData::spring_echomixer_clean_wavSize, BinaryData::spring_schaller_clean_wavSize,
        BinaryData::spring_pioneer_clean_wavSize };
    // DECAY changes the usable physical IR length, rather than merely turning
    // its output up. Quantisation avoids continuously rebuilding convolution.
    const auto decaySeconds = juce::jmap (static_cast<float> (decayStep) / 15.0f, 0.45f, 5.8f);
    const auto maximumIrSamples = static_cast<size_t> (sampleRate * decaySeconds);
    convolution.loadImpulseResponse (data[index], static_cast<size_t> (sizes[index]),
        juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, maximumIrSamples,
        juce::dsp::Convolution::Normalise::yes);
    loadedImpulse = index;
    loadedDecayStep = decayStep;
}

void SpringModule::process (juce::AudioBuffer<float>& buffer)
{
    if (! wetMix.isSmoothing() && wetMix.getCurrentValue() <= 0.00001f)
        return;

    const auto samples = buffer.getNumSamples();
    const auto channels = juce::jmin (buffer.getNumChannels(), channelCount);
    if (samples > maximumBlockSize || channels == 0) return;
    for (int channel = 0; channel < channels; ++channel)
        wetBuffer.copyFrom (channel, 0, buffer, channel, 0, samples);

    const auto dwellGain = 1.0f + dwellAmount * 5.0f;
    for (int channel = 0; channel < channels; ++channel)
        for (int sample = 0; sample < samples; ++sample)
        {
            const auto input = wetBuffer.getSample (channel, sample);
            const auto driven = std::tanh (input * dwellGain) / dwellGain;
            wetBuffer.setSample (channel, sample,
                juce::jmap (dwellAmount, input, driven) * (1.0f + dwellAmount * 0.55f));
        }

    juce::dsp::AudioBlock<float> wetBlock (wetBuffer);
    auto activeWet = wetBlock.getSubBlock (0, static_cast<size_t> (samples));
    juce::dsp::ProcessContextReplacing<float> wetContext (activeWet);
    convolution.process (wetContext);
    for (int channel = 0; channel < channels; ++channel)
        dripBuffer.copyFrom (channel, 0, wetBuffer, channel, 0, samples);
    juce::dsp::AudioBlock<float> dripBlock (dripBuffer);
    auto activeDrip = dripBlock.getSubBlock (0, static_cast<size_t> (samples));
    juce::dsp::ProcessContextReplacing<float> dripContext (activeDrip);
    dripFilter.process (dripContext);
    bodyFilter.process (wetContext);
    toneFilter.process (wetContext);

    // Normalised IRs already carry ample tail energy. Avoid the previous gain
    // boost that made long decays swamp the source as Mix increased.
    const auto tailGain = juce::jmap (decayAmount, 0.78f, 1.00f);
    const auto envelopeCoefficient = 1.0f - std::exp (-1.0f /
        (static_cast<float> (sampleRate) * 0.028f));
    for (int sample = 0; sample < samples; ++sample)
    {
        const auto mix = wetMix.getNextValue();
        const auto dryGain = std::cos (mix * juce::MathConstants<float>::halfPi);
        const auto wetGain = std::sin (mix * juce::MathConstants<float>::halfPi) * 0.78f;
        for (int channel = 0; channel < channels; ++channel)
        {
            const auto dry = buffer.getSample (channel, sample);
            auto& inputEnv = inputEnvelope[static_cast<size_t> (channel)];
            const auto magnitude = std::abs (dry);
            inputEnv += envelopeCoefficient * (magnitude - inputEnv);
            const auto transient = juce::jlimit (0.0f, 1.0f, (magnitude - inputEnv) * 9.0f);
            // Drip is an onset accent, not a permanent bright EQ on the tail.
            const auto dripGain = dripAmount * (0.18f + transient * 1.35f);
            const auto wet = (wetBuffer.getSample (channel, sample)
                            + dripBuffer.getSample (channel, sample) * dripGain) * tailGain;
            // A small amount of input-aware ducking keeps pick attack and note
            // body forward; the spring naturally blooms as the note relaxes.
            const auto ducking = 1.0f - juce::jlimit (0.0f, 1.0f, inputEnv * 5.0f) * 0.30f;
            buffer.setSample (channel, sample, dry * dryGain + wet * wetGain * ducking);
        }
    }
}
