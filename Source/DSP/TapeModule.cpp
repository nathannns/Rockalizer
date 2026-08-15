#include "TapeModule.h"

void TapeModule::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    const auto channels = static_cast<int> (spec.numChannels);
    wowBuffer.setSize (channels, static_cast<int> (sampleRate * 0.04) + 4);
    envelope.assign (channels, 0.0f); toneState.assign (channels, 0.0f);
    detectorLowState.assign (channels, 0.0f); magnetisationState.assign (channels, 0.0f);
    bassState.assign (channels, 0.0f); midState.assign (channels, 0.0f);
    wetMix.reset (sampleRate, 0.02);
    reset();
}

void TapeModule::reset()
{
    wowBuffer.clear(); writeIndex = 0; lfoPhase = 0.0f;
    std::fill (envelope.begin(), envelope.end(), 0.0f);
    std::fill (detectorLowState.begin(), detectorLowState.end(), 0.0f);
    std::fill (magnetisationState.begin(), magnetisationState.end(), 0.0f);
    std::fill (toneState.begin(), toneState.end(), 0.0f);
    std::fill (bassState.begin(), bassState.end(), 0.0f);
    std::fill (midState.begin(), midState.end(), 0.0f);
    wetMix.setCurrentAndTargetValue (0.0f);
}

void TapeModule::setParameters (float drive, float compression, float tone, float age,
                                float mix, bool enabled, int type)
{
    const auto normalisedDrive = juce::jlimit (0.0f, 1.0f, drive * 0.01f);
    // A slower lower half gives useful clean headroom. The nonlinearity itself
    // is level-dependent, so no artificial playing-strength gate is needed.
    driveValue = std::pow (normalisedDrive, 1.05f);
    compValue = std::pow (juce::jlimit (0.0f, 1.0f, compression * 0.01f), 0.82f);
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
    if (! wetMix.isSmoothing() && wetMix.getCurrentValue() <= 0.00001f)
        return;

    const auto channels = juce::jmin (buffer.getNumChannels(), wowBuffer.getNumChannels());
    const auto cassetteMode = tapeType == cassette;
    // COMP is deliberately the final stage of this module, after the tape
    // blend and immediately before Chorus in the processor chain.
    const auto thresholdDb = (cassetteMode ? -12.0f : -10.0f) - compValue * 12.0f;
    const auto ratio = 1.0f + compValue * 4.5f;
    const auto makeupDb = compValue * (cassetteMode ? 10.0f : 11.0f);
    constexpr float kneeDb = 8.0f;
    const auto cutoffTop = cassetteMode ? 13500.0f : 20000.0f;
    const auto cutoffBottom = cassetteMode ? 2500.0f : 5000.0f;
    const auto cutoff = juce::jmap (toneValue, cutoffBottom, cutoffTop)
                      * (1.0f - ageValue * (cassetteMode ? 0.68f : 0.42f));
    const auto toneCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                   * cutoff / static_cast<float> (sampleRate));
    const auto bassCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                   * (cassetteMode ? 130.0f : 85.0f)
                                                   / static_cast<float> (sampleRate));
    const auto midCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                  * 1250.0f / static_cast<float> (sampleRate));
    const auto detectorLowCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                          * 120.0f / static_cast<float> (sampleRate));
    const auto magnetisationCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                            * (cassetteMode ? 5200.0f : 7800.0f)
                                                            / static_cast<float> (sampleRate));
    const auto attackSeconds = juce::jmap (compValue, 0.014f, 0.006f);
    const auto releaseSeconds = juce::jmap (compValue, 0.145f, 0.300f);
    const auto attack = std::exp (-1.0f / (static_cast<float> (sampleRate) * attackSeconds));
    const auto release = std::exp (-1.0f / (static_cast<float> (sampleRate) * releaseSeconds));
    const auto wowRate = cassetteMode ? 0.75f : 0.32f;
    // Keep wow subtle. Large values mixed with the dry path create audible
    // comb-filter amplitude movement that can be mistaken for tremolo.
    const auto wowDepthMs = std::pow (ageValue, 0.78f) * (cassetteMode ? 1.15f : 0.28f);
    const auto baseDelay = static_cast<float> (sampleRate) * 0.008f;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto mix = wetMix.getNextValue();
        for (int channel = 0; channel < channels; ++channel)
        {
            const auto dry = buffer.getSample (channel, sample);

            wowBuffer.setSample (channel, writeIndex, dry);
            // Both channels share the same transport motion. A stereo phase
            // offset made mono guitars wander from left to right.
            const auto wow = std::sin (lfoPhase) * wowDepthMs * 0.001f
                           * static_cast<float> (sampleRate);
            auto x = readWow (channel, baseDelay + wow);
            // Studio tape has more clean headroom than cassette, followed by a
            // broad magnetic knee rather than a sudden distortion threshold.
            // The extra 3 dB record level and stronger flux curve make Drive
            // clearly audible without turning it into an ordinary gain knob.
            const auto inputDb = driveValue * (cassetteMode ? 28.0f : 25.0f);
            const auto inputGain = juce::Decibels::decibelsToGain (inputDb);
            const auto driven = x * inputGain;
            const auto preSaturationBody = std::tanh (driven * 0.55f)
                                         * juce::Decibels::decibelsToGain (-inputDb);
            const auto distortionAmount = std::pow (driveValue, 1.25f);
            const auto bias = (cassetteMode ? 0.045f : 0.018f)
                            + distortionAmount * (cassetteMode ? 0.050f : 0.028f);
            const auto shape = 1.0f + distortionAmount * (cassetteMode ? 2.85f : 2.48f)
                                     + ageValue * (cassetteMode ? 0.45f : 0.24f);
            const auto biasTanh = std::tanh (bias);
            const auto smallSignalSlope = shape * (1.0f - biasTanh * biasTanh);
            const auto anhysteretic = (std::tanh (driven * shape + bias) - biasTanh)
                                    / juce::jmax (0.1f, smallSignalSlope);
            auto& magnetisation = magnetisationState[static_cast<size_t> (channel)];
            magnetisation += magnetisationCoefficient * (anhysteretic - magnetisation);
            const auto memoryMix = (cassetteMode ? 0.16f : 0.115f) + ageValue * 0.08f;
            x = anhysteretic * (1.0f - memoryMix) + magnetisation * memoryMix;
            // Cancel the record gain for small signals. Drive therefore lowers
            // headroom instead of acting like a volume knob; only peaks that
            // approach the magnetic ceiling are compressed and saturated.
            x *= juce::Decibels::decibelsToGain (-inputDb);

            auto& lp = toneState[static_cast<size_t> (channel)];
            lp += toneCoefficient * (x - lp);
            auto& bass = bassState[static_cast<size_t> (channel)];
            bass += bassCoefficient * (preSaturationBody - bass);
            auto& mids = midState[static_cast<size_t> (channel)];
            mids += midCoefficient * (preSaturationBody - mids);
            // Preserve body as Tone moves bright, while Age adds an audible
            // head-bump and increasingly worn, softened character.
            const auto brightBody = juce::jlimit (0.0f, 1.0f, (toneValue - 0.52f) / 0.48f);
            const auto driveBody = distortionAmount;
            const auto midBody = brightBody * (cassetteMode ? 0.14f : 0.11f)
                               + driveBody * (cassetteMode ? 0.10f : 0.13f);
            const auto ageBump = ageValue * (cassetteMode ? 0.14f : 0.10f);
            const auto driveBass = driveBody * (cassetteMode ? 0.16f : 0.20f);
            x = lp + mids * midBody
                   + bass * ((cassetteMode ? 0.035f : 0.085f) + ageBump + driveBass);
            // Tape Mix first, then the one-knob compressor. Its detector is
            // high-pass weighted so low fundamentals do not collapse; a small
            // parallel low-band feed restores guitar body and pick "thump".
            auto compressed = dry * (1.0f - mix) + x * mix;
            auto& detectorLow = detectorLowState[static_cast<size_t> (channel)];
            detectorLow += detectorLowCoefficient * (compressed - detectorLow);
            const auto detectorSample = compressed - detectorLow * 0.76f;
            const auto level = std::abs (detectorSample);
            auto& env = envelope[static_cast<size_t> (channel)];
            env = level > env ? attack * env + (1.0f - attack) * level
                              : release * env + (1.0f - release) * level;
            const auto envelopeDb = juce::Decibels::gainToDecibels (env, -120.0f);
            const auto overDb = envelopeDb - thresholdDb;
            float reductionDb = 0.0f;
            if (overDb >= kneeDb * 0.5f)
                reductionDb = overDb * (1.0f - 1.0f / ratio);
            else if (overDb > -kneeDb * 0.5f)
            {
                const auto kneePosition = overDb + kneeDb * 0.5f;
                reductionDb = (1.0f - 1.0f / ratio)
                            * kneePosition * kneePosition / (2.0f * kneeDb);
            }
            const auto makeupActivity = juce::jlimit (0.0f, 1.0f,
                                                       (envelopeDb + 72.0f) / 30.0f);
            compressed *= juce::Decibels::decibelsToGain (
                makeupDb * makeupActivity - reductionDb + compValue * makeupActivity * 2.0f);
            compressed += detectorLow * compValue * 0.16f;
            buffer.setSample (channel, sample, compressed);
        }
        writeIndex = (writeIndex + 1) % wowBuffer.getNumSamples();
        lfoPhase += juce::MathConstants<float>::twoPi * wowRate / static_cast<float> (sampleRate);
        if (lfoPhase >= juce::MathConstants<float>::twoPi) lfoPhase -= juce::MathConstants<float>::twoPi;
    }
}
