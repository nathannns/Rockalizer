#include "SpringModule.h"

void SpringModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maximumBlockSize = static_cast<int> (spec.maximumBlockSize);
    channelCount = static_cast<int> (spec.numChannels);
    wetBuffer.setSize (channelCount, maximumBlockSize);
    dripBuffer.setSize (channelCount, maximumBlockSize);
    tailBuffer.setSize (channelCount, static_cast<int> (sampleRate * 0.055) + 4);
    tailBuffer.clear();
    convolution.prepare (spec); toneFilter.prepare (spec); bodyFilter.prepare (spec); dripFilter.prepare (spec);
    toneFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    bodyFilter.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    bodyFilter.setCutoffFrequency (115.0f);
    dripFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    dripFilter.setCutoffFrequency (2450.0f); dripFilter.setResonance (0.76f);
    dripDetectorCoefficient = 1.0f - std::exp (-1.0f /
        (static_cast<float> (sampleRate) * 0.018f));
    envelopeAttack = 1.0f - std::exp (-1.0f /
        (static_cast<float> (sampleRate) * 0.006f));
    envelopeRelease = 1.0f - std::exp (-1.0f /
        (static_cast<float> (sampleRate) * 0.220f));
    cachedToneHz = -1.0f;
    inputEnvelope.assign (static_cast<size_t> (channelCount), 0.0f);
    dripEnvelope.assign (static_cast<size_t> (channelCount), 0.0f);
    tailDampingState.assign (static_cast<size_t> (channelCount), 0.0f);
    wetMix.reset (spec.sampleRate, 0.03);
    reset();
    triggerAsyncUpdate();
}

void SpringModule::reset()
{
    convolution.reset(); toneFilter.reset(); bodyFilter.reset(); dripFilter.reset();
    std::fill (inputEnvelope.begin(), inputEnvelope.end(), 0.0f);
    std::fill (dripEnvelope.begin(), dripEnvelope.end(), 0.0f);
    std::fill (tailDampingState.begin(), tailDampingState.end(), 0.0f);
    tailBuffer.clear();
    tailWriteIndex = 0;
    wetMix.setCurrentAndTargetValue (0.0f);
}

void SpringModule::setParameters (float decay, float dwell, float tone, float drip, float mix,
                                  bool enabled, int impulseIndex)
{
    decayAmount = juce::jlimit (0.0f, 1.0f, decay * 0.01f);
    dwellAmount = juce::jlimit (0.0f, 1.0f, dwell * 0.01f);
    dripAmount = juce::jlimit (0.0f, 1.0f, drip * 0.01f);
    const auto toneHz = juce::jmap (juce::jlimit (0.0f, 1.0f, tone * 0.01f),
                                    1400.0f, 12500.0f);
    if (std::abs (toneHz - cachedToneHz) > 0.01f)
    {
        cachedToneHz = toneHz;
        toneFilter.setCutoffFrequency (toneHz);
    }
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
    // DECAY changes the physical IR length. Decode it here on the background
    // thread and taper the ending before convolution so shortened tanks never
    // terminate at a hard sample boundary.
    const auto decaySeconds = juce::jmap (static_cast<float> (decayStep) / 15.0f, 0.45f, 5.8f);
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatReader> reader (wav.createReaderFor (
        new juce::MemoryInputStream (data[index], static_cast<size_t> (sizes[index]), false), true));
    if (reader != nullptr)
    {
        const auto length = static_cast<int> (juce::jmin (
            static_cast<juce::int64> (reader->lengthInSamples),
            static_cast<juce::int64> (reader->sampleRate * decaySeconds)));
        const auto channels = juce::jlimit (1, 2, static_cast<int> (reader->numChannels));
        juce::AudioBuffer<float> impulse (channels, juce::jmax (1, length));
        reader->read (&impulse, 0, length, 0, true, channels > 1);
        const auto fadeSamples = juce::jmin (length, juce::roundToInt (
            reader->sampleRate * juce::jmap (static_cast<float> (decayStep) / 15.0f,
                                             0.10f, 0.22f)));
        for (int sample = juce::jmax (0, length - fadeSamples); sample < length; ++sample)
        {
            const auto position = static_cast<float> (length - sample)
                                / static_cast<float> (juce::jmax (1, fadeSamples));
            const auto taper = std::sin (position * juce::MathConstants<float>::halfPi);
            impulse.applyGain (sample, 1, taper * taper);
        }
        convolution.loadImpulseResponse (std::move (impulse), reader->sampleRate,
            juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes,
            juce::dsp::Convolution::Normalise::yes);
    }
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

    // Dwell drives the spring transducer before the IR. A parallel clean path
    // keeps low settings open, while the asymmetric soft clip adds the dense,
    // slightly compressed excitation of a harder-driven tank.
    if (dwellAmount > 0.0001f)
    {
        const auto dwellGain = 1.0f + dwellAmount * 3.6f;
        for (int channel = 0; channel < channels; ++channel)
            for (int sample = 0; sample < samples; ++sample)
            {
                const auto input = wetBuffer.getSample (channel, sample);
                const auto bias = 0.035f * dwellAmount;
                const auto dc = std::tanh (bias * dwellGain);
                const auto driven = (std::tanh ((input + bias) * dwellGain) - dc)
                                  / juce::jmax (1.0f, dwellGain * 0.72f);
                wetBuffer.setSample (channel, sample, juce::jmap (dwellAmount * 0.72f,
                                                                  input, driven)
                                                   * (1.0f + dwellAmount * 0.24f));
            }
    }

    // Drip is also an excitation of the tank, not a bright layer pasted onto
    // the convolved output. Detect pick onsets, shape them through the resonant
    // band and feed that energy into the same IR as the main guitar signal.
    const auto dripEnabled = dripAmount > 0.0001f;
    if (dripEnabled)
    {
        for (int channel = 0; channel < channels; ++channel)
        {
            for (int sample = 0; sample < samples; ++sample)
            {
                const auto input = wetBuffer.getSample (channel, sample);
                auto& envelope = dripEnvelope[static_cast<size_t> (channel)];
                const auto magnitude = std::abs (input);
                envelope += dripDetectorCoefficient * (magnitude - envelope);
                const auto onset = juce::jlimit (0.0f, 1.0f, (magnitude - envelope) * 8.0f);
                dripBuffer.setSample (channel, sample, input * onset);
            }
        }
        juce::dsp::AudioBlock<float> dripBlock (dripBuffer);
        auto activeDrip = dripBlock.getSubBlock (0, static_cast<size_t> (samples));
        juce::dsp::ProcessContextReplacing<float> dripContext (activeDrip);
        dripFilter.process (dripContext);
        for (int channel = 0; channel < channels; ++channel)
            wetBuffer.addFrom (channel, 0, dripBuffer, channel, 0, samples,
                               dripAmount * 0.82f);
    }

    juce::dsp::AudioBlock<float> wetBlock (wetBuffer);
    auto activeWet = wetBlock.getSubBlock (0, static_cast<size_t> (samples));
    juce::dsp::ProcessContextReplacing<float> wetContext (activeWet);
    convolution.process (wetContext);
    bodyFilter.process (wetContext);
    toneFilter.process (wetContext);

    // A quiet cross-coupled mechanical tail bridges the quantised end of a
    // shortened IR. This turns an obvious cutoff into a soft two-stage decay,
    // without rebuilding convolution or adding a long artificial wash.
    const auto tailSize = tailBuffer.getNumSamples();
    const auto tailFeedback = 0.30f + decayAmount * 0.38f;
    const auto tailDamping = 0.10f + decayAmount * 0.04f;
    for (int sample = 0; sample < samples; ++sample)
    {
        float delayed[2] {};
        for (int channel = 0; channel < channels; ++channel)
        {
            const auto delayMs = channel == 0 ? 31.0f : 43.0f;
            const auto delaySamples = juce::jmin (tailSize - 1,
                juce::roundToInt (static_cast<float> (sampleRate) * delayMs * 0.001f));
            const auto readIndex = (tailWriteIndex - delaySamples + tailSize) % tailSize;
            delayed[channel] = tailBuffer.getSample (channel, readIndex);
        }
        for (int channel = 0; channel < channels; ++channel)
        {
            const auto wet = wetBuffer.getSample (channel, sample);
            const auto other = channels > 1 ? 1 - channel : channel;
            auto& damped = tailDampingState[static_cast<size_t> (channel)];
            damped += tailDamping * (delayed[other] - damped);
            const auto regeneration = damped * tailFeedback;
            tailBuffer.setSample (channel, tailWriteIndex, std::tanh (wet + regeneration));
            wetBuffer.setSample (channel, sample, wet + damped * 0.105f);
        }
        tailWriteIndex = (tailWriteIndex + 1) % tailSize;
    }

    // Normalised IRs already carry ample tail energy. Avoid the previous gain
    // boost that made long decays swamp the source as Mix increased.
    const auto tailGain = juce::jmap (decayAmount, 0.82f, 1.05f);
    const auto mixIsSmoothing = wetMix.isSmoothing();
    const auto steadyMix = wetMix.getCurrentValue();
    // Rockalizer is an insert-style guitar effect, not a 100%-wet aux return.
    // Preserve enough direct signal at high Mix for the note and pick attack
    // to remain present while the spring grows into a dense wash.
    const auto mixGains = [] (float mix)
    {
        const auto shapedMix = std::pow (juce::jlimit (0.0f, 1.0f, mix), 0.78f);
        return std::pair { 1.0f - mix * 0.62f, shapedMix * 0.86f };
    };
    const auto steadyGains = mixGains (steadyMix);
    const auto steadyDryGain = steadyGains.first;
    const auto steadyWetGain = steadyGains.second;
    for (int sample = 0; sample < samples; ++sample)
    {
        const auto mix = mixIsSmoothing ? wetMix.getNextValue() : steadyMix;
        const auto gains = mixIsSmoothing ? mixGains (mix)
                                          : std::pair { steadyDryGain, steadyWetGain };
        const auto dryGain = gains.first;
        const auto wetGain = gains.second;
        for (int channel = 0; channel < channels; ++channel)
        {
            const auto dry = buffer.getSample (channel, sample);
            auto& inputEnv = inputEnvelope[static_cast<size_t> (channel)];
            const auto magnitude = std::abs (dry);
            const auto envelopeCoefficient = magnitude > inputEnv ? envelopeAttack : envelopeRelease;
            inputEnv += envelopeCoefficient * (magnitude - inputEnv);
            const auto wet = wetBuffer.getSample (channel, sample) * tailGain;
            // A small amount of input-aware ducking keeps pick attack and note
            // body forward; the spring naturally blooms as the note relaxes.
            const auto ducking = 1.0f - juce::jlimit (0.0f, 1.0f, inputEnv * 4.5f) * 0.38f;
            buffer.setSample (channel, sample, dry * dryGain + wet * wetGain * ducking);
        }
    }
}
