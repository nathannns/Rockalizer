#include <JuceHeader.h>
#include "DSP/ChorusModule.h"
#include "DSP/EchoModule.h"
#include "DSP/SpringModule.h"
#include "DSP/TapeModule.h"
#include "DSP/TremoloModule.h"
#include <iostream>

namespace
{
bool isFiniteAndBounded (const juce::AudioBuffer<float>& buffer, float limit = 8.01f)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto value = buffer.getSample (channel, sample);
            if (! std::isfinite (value) || std::abs (value) > limit)
                return false;
        }
    return true;
}

void fillTestSignal (juce::AudioBuffer<float>& buffer, double sampleRate, int64_t offset)
{
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto phase = juce::MathConstants<double>::twoPi * 220.0
                         * static_cast<double> (offset + sample) / sampleRate;
        const auto value = static_cast<float> (std::sin (phase) * 0.22);
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample (channel, sample, value);
    }
}

bool runConfiguration (double sampleRate, int blockSize, int channels)
{
    const juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (blockSize),
                                       static_cast<juce::uint32> (channels) };
    ChorusModule chorus; EchoModule echo; TapeModule tape; TremoloModule tremolo;
    chorus.prepare (spec); echo.prepare (spec); tape.prepare (spec); tremolo.prepare (spec);
    juce::AudioBuffer<float> buffer (channels, blockSize);
    int64_t offset = 0;
    std::vector<float> previousSamples (static_cast<size_t> (channels), 0.0f);

    for (int block = 0; block < 240; ++block)
    {
        fillTestSignal (buffer, sampleRate, offset); offset += blockSize;
        tape.setParameters (55.0f, 45.0f, 60.0f, 40.0f, 65.0f, true, 1, 1);
        tape.process (buffer);
        tremolo.setAmount (65.0f); tremolo.process (buffer);
        chorus.setParameters (0.55f, 70.0f, 90.0f, 8500.0f, 45.0f, true, block % 80 >= 40);
        chorus.process (buffer);
        echo.setParameters (265.0f, 82.0f, 6200.0f, 35.0f, 40.0f, 90.0f, true, 3);
        echo.process (buffer);
        if (! isFiniteAndBounded (buffer)) return false;

        // At high Mix the delay path used to expose a hard limiter edge as a
        // single-sample click. Keep this intentionally generous so normal
        // guitar transients pass while discontinuities fail the matrix.
        if (block > 4)
            for (int channel = 0; channel < channels; ++channel)
                for (int sample = 0; sample < blockSize; ++sample)
                {
                    const auto value = buffer.getSample (channel, sample);
                    if (std::abs (value - previousSamples[static_cast<size_t> (channel)]) > 2.0f)
                        return false;
                    previousSamples[static_cast<size_t> (channel)] = value;
                }
        else
            for (int channel = 0; channel < channels; ++channel)
                previousSamples[static_cast<size_t> (channel)] = buffer.getSample (channel, blockSize - 1);
    }

    buffer.clear();
    tape.process (buffer); tremolo.process (buffer); chorus.process (buffer); echo.process (buffer);
    return isFiniteAndBounded (buffer);
}

bool springRemainsStable (double sampleRate, int blockSize, int channels)
{
    const juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (blockSize),
                                       static_cast<juce::uint32> (channels) };
    SpringModule spring;
    spring.prepare (spec);
    juce::AudioBuffer<float> buffer (channels, blockSize);
    int64_t offset = 0;
    std::vector<float> previousSamples (static_cast<size_t> (channels), 0.0f);

    for (int block = 0; block < 240; ++block)
    {
        fillTestSignal (buffer, sampleRate, offset); offset += blockSize;
        // Exercise all 3 tank models across the sweep, with dwell/drip/decay
        // all active -- the allpass dispersion, drive saturation, and damped
        // feedback matrix are all live at once, the worst case for any
        // interaction between them.
        spring.setParameters (72.0f, 60.0f, 55.0f, 50.0f, 70.0f, true, block % 3);
        spring.process (buffer);
        if (! isFiniteAndBounded (buffer)) return false;

        if (block > 4)
            for (int channel = 0; channel < channels; ++channel)
                for (int sample = 0; sample < blockSize; ++sample)
                {
                    const auto value = buffer.getSample (channel, sample);
                    if (std::abs (value - previousSamples[static_cast<size_t> (channel)]) > 2.0f)
                        return false;
                    previousSamples[static_cast<size_t> (channel)] = value;
                }
        else
            for (int channel = 0; channel < channels; ++channel)
                previousSamples[static_cast<size_t> (channel)] = buffer.getSample (channel, blockSize - 1);
    }

    buffer.clear();
    spring.process (buffer);
    return isFiniteAndBounded (buffer);
}

bool echoRemainsCentred (double sampleRate, int blockSize)
{
    const juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (blockSize), 2 };
    EchoModule echo;
    echo.prepare (spec);
    juce::AudioBuffer<float> buffer (2, blockSize);
    int64_t offset = 0;
    const auto blocksToCrossWobbleWrap = static_cast<int> (std::ceil (sampleRate * 2.1
                                                    / static_cast<double> (blockSize)));
    float previous = 0.0f;
    for (int block = 0; block < blocksToCrossWobbleWrap; ++block)
    {
        fillTestSignal (buffer, sampleRate, offset);
        offset += blockSize;
        echo.setParameters (180.0f, 75.0f, 7000.0f, 85.0f, 20.0f, 100.0f, true, 3);
        echo.process (buffer);
        for (int sample = 0; sample < blockSize; ++sample)
        {
            if (std::abs (buffer.getSample (0, sample) - buffer.getSample (1, sample)) > 0.0001f)
                return false;
            const auto value = buffer.getSample (0, sample);
            if (block > 0 && std::abs (value - previous) > 0.08f)
                return false;
            previous = value;
        }
    }
    return true;
}

bool modulationIsPhaseSafe (double sampleRate, int blockSize, int flangerMode)
{
    const juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (blockSize), 2 };
    ChorusModule chorus;
    chorus.prepare (spec);
    juce::AudioBuffer<float> buffer (2, blockSize);
    int64_t offset = 0;
    double leftEnergy = 0.0, rightEnergy = 0.0, monoEnergy = 0.0, inputEnergy = 0.0;
    for (int block = 0; block < 180; ++block)
    {
        fillTestSignal (buffer, sampleRate, offset);
        offset += blockSize;
        for (int sample = 0; sample < blockSize; ++sample)
            inputEnergy += std::pow (buffer.getSample (0, sample), 2.0f);
        chorus.setParameters (flangerMode == 0 ? 0.32f : 1.2f, 70.0f, 90.0f,
                              8000.0f, 45.0f, true, flangerMode);
        chorus.process (buffer);
        if (block < 8) continue;
        for (int sample = 0; sample < blockSize; ++sample)
        {
            const auto left = buffer.getSample (0, sample);
            const auto right = buffer.getSample (1, sample);
            leftEnergy += left * left;
            rightEnergy += right * right;
            const auto mono = 0.5f * (left + right);
            monoEnergy += mono * mono;
        }
    }
    const auto balanceDb = std::abs (juce::Decibels::gainToDecibels (
        static_cast<float> (std::sqrt ((leftEnergy + 1.0e-12) / (rightEnergy + 1.0e-12)))));
    const auto monoRatio = monoEnergy / juce::jmax (inputEnergy, 1.0e-12);
    return balanceDb < 1.0f && monoRatio > 0.22;
}
}

int main()
{
    const double sampleRates[] { 44100.0, 48000.0, 88200.0, 96000.0 };
    const int blockSizes[] { 32, 64, 128, 256, 512, 1024 };
    for (const auto sampleRate : sampleRates)
        for (const auto blockSize : blockSizes)
        {
            for (const auto flangerMode : { 0, 1, 2, 3 })
                if (! modulationIsPhaseSafe (sampleRate, blockSize, flangerMode))
                {
                    std::cerr << "FAILED modulation phase: mode " << flangerMode << ", "
                              << sampleRate << " Hz, " << blockSize << " samples\n";
                    return 1;
                }
            if (! echoRemainsCentred (sampleRate, blockSize))
            {
                std::cerr << "FAILED centred Echo: " << sampleRate << " Hz, "
                          << blockSize << " samples\n";
                return 1;
            }
            for (const auto channels : { 1, 2 })
            {
                if (! runConfiguration (sampleRate, blockSize, channels))
                {
                    std::cerr << "FAILED: " << sampleRate << " Hz, " << blockSize
                              << " samples, " << channels << " channels\n";
                    return 1;
                }
                if (! springRemainsStable (sampleRate, blockSize, channels))
                {
                    std::cerr << "FAILED Spring: " << sampleRate << " Hz, " << blockSize
                              << " samples, " << channels << " channels\n";
                    return 1;
                }
            }
        }
    std::cout << "Rockalizer DSP validation passed\n";
    return 0;
}
