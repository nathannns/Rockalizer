#include <JuceHeader.h>
#include "DSP/ChorusModule.h"
#include "DSP/EchoModule.h"
#include "DSP/SpringModule.h"
#include "DSP/TapeModule.h"
#include "DSP/TremoloModule.h"
#include <atomic>
#include <iostream>
#include <thread>

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
        tape.setParameters (55.0f, 45.0f, 60.0f, 40.0f, 65.0f, 100.0f, true, 1, 1);
        tape.process (buffer);
        tremolo.setAmount (65.0f); tremolo.process (buffer);
        chorus.setParameters (0.55f, 70.0f, 90.0f, 8500.0f, 45.0f, true, block % 80 >= 40);
        chorus.process (buffer);
        echo.setParameters (265.0f, 82.0f, 40.0f, 60.0f, 35.0f, 40.0f, 90.0f, true, 3);
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

bool tremoloHarmonicVoiceIsStable (double sampleRate, int blockSize, int channels)
{
    const juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (blockSize),
                                       static_cast<juce::uint32> (channels) };
    TremoloModule tremolo;
    tremolo.prepare (spec);
    tremolo.setVoice (TremoloModule::Voice::harmonic);
    juce::AudioBuffer<float> buffer (channels, blockSize);
    int64_t offset = 0;
    bool producedStereoSpread = false;

    for (int block = 0; block < 240; ++block)
    {
        fillTestSignal (buffer, sampleRate, offset); offset += blockSize;
        // Sweep depth and rate so the raised-cosine AM + Linkwitz-Riley
        // crossover run across their full ranges; the crossover is a
        // bounded linear filter and the gains stay in [1-depth, 1+depth],
        // so this is a NaN/Inf/blow-up check, not a tuning assertion.
        tremolo.setAmount (static_cast<float> (10 + (block * 37) % 90));
        tremolo.setRate (0.5f + 0.05f * static_cast<float> (block % 128));
        tremolo.process (buffer);
        if (! isFiniteAndBounded (buffer)) return false;

        // The harmonic voice swaps the low/high gain assignment between the
        // two channels (see TremoloModule.h), so a mono test signal must NOT
        // come out identical left and right at any non-zero depth -- that
        // difference is exactly what makes it stereo rather than dual-mono.
        if (channels == 2 && ! producedStereoSpread)
            for (int sample = 0; sample < blockSize; ++sample)
                if (std::abs (buffer.getSample (0, sample) - buffer.getSample (1, sample)) > 0.0001f)
                {
                    producedStereoSpread = true;
                    break;
                }
    }

    return channels == 1 || producedStereoSpread;
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

bool springAsyncLoadBoundaryIsStable()
{
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 128;
    constexpr int totalBlocks = 45000; // two minutes of audio, rendered faster than real time
    SpringModule spring;
    spring.prepare ({ sampleRate, blockSize, 2 });

    std::atomic<bool> stable { true };
    std::atomic<float> maximumStep { 0.0f };
    std::atomic<float> maximumMagnitude { 0.0f };
    std::atomic<int> failedBlock { -1 };
    std::thread audioThread ([&]
    {
        juce::AudioBuffer<float> buffer (2, blockSize);
        int64_t offset = 0;
        float previous[2] {};
        for (int block = 0; block < totalBlocks && stable.load (std::memory_order_relaxed); ++block)
        {
            fillTestSignal (buffer, sampleRate, offset);
            offset += blockSize;
            // Switch tanks every simulated half-second. That exercises the
            // asynchronous Convolution publication boundary hundreds of
            // times, far more aggressively than normal UI use.
            const auto model = (block / 188) % 3;
            spring.setParameters (78.0f, 62.0f, 58.0f, 55.0f, 72.0f, true, model);
            spring.process (buffer);

            for (int channel = 0; channel < 2; ++channel)
                for (int sample = 0; sample < blockSize; ++sample)
                {
                    const auto value = buffer.getSample (channel, sample);
                    const auto magnitude = std::abs (value);
                    auto observedMagnitude = maximumMagnitude.load (std::memory_order_relaxed);
                    while (magnitude > observedMagnitude
                           && ! maximumMagnitude.compare_exchange_weak (observedMagnitude, magnitude,
                                                                        std::memory_order_relaxed)) {}
                    if (! std::isfinite (value) || magnitude > 8.01f)
                    {
                        failedBlock.store (block, std::memory_order_relaxed);
                        stable.store (false, std::memory_order_relaxed);
                        break;
                    }
                    const auto step = std::abs (value - previous[channel]);
                    auto observed = maximumStep.load (std::memory_order_relaxed);
                    while (step > observed
                           && ! maximumStep.compare_exchange_weak (observed, step,
                                                                   std::memory_order_relaxed)) {}
                    if (block > 8 && step > 2.0f)
                    {
                        stable.store (false, std::memory_order_relaxed);
                        break;
                    }
                    previous[channel] = value;
                }

            if ((block & 31) == 0)
                std::this_thread::yield();
        }
        juce::MessageManager::callAsync ([]
        {
            juce::MessageManager::getInstance()->stopDispatchLoop();
        });
    });

    // AsyncUpdater callbacks and Convolution's queued IR publication happen
    // on the message thread. Pump it while the synthetic audio callback runs
    // so this test covers the actual cross-thread transition.
    juce::MessageManager::getInstance()->runDispatchLoop();
    audioThread.join();

    std::cout << "Spring async-load soak maximum sample step: "
              << maximumStep.load (std::memory_order_relaxed)
              << ", maximum magnitude: " << maximumMagnitude.load (std::memory_order_relaxed)
              << ", failed block: " << failedBlock.load (std::memory_order_relaxed) << '\n';
#if defined(ROCKALIZER_SPRING_ANALYSIS)
    const auto stageMaxima = spring.getAnalysisStageMaxima();
    std::cout << "Spring stage maxima (convolution/dispersion/tail/delayed/write/damped/fraction/buffer): "
              << stageMaxima[0] << ", " << stageMaxima[1] << ", " << stageMaxima[2]
              << ", " << stageMaxima[3] << ", " << stageMaxima[4] << ", " << stageMaxima[5]
              << ", " << stageMaxima[6] << ", " << stageMaxima[7] << '\n';
#endif
    return stable.load (std::memory_order_relaxed);
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
        echo.setParameters (180.0f, 75.0f, 40.0f, 60.0f, 85.0f, 20.0f, 100.0f, true, 3);
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
    // Chorus mode's antiphase stereo decorrelation is authentic to the real
    // SDD-320 (see ChorusModule::process), which genuinely does put more
    // energy on one channel than the other for multi-second stretches at its
    // ~0.2-0.5Hz LFO rate -- that's the real unit's "3D" character, not a
    // defect, and only converges to a balanced average once measured over a
    // window comparable to the LFO period. A fixed block *count* would make
    // that window scale with blockSize instead, so it's driven by real time:
    // one second of warm-up, twelve seconds of measurement, regardless of
    // how the audio happens to be chunked.
    const auto warmupBlocks = static_cast<int> (1.0 * sampleRate / blockSize);
    const auto totalBlocks = static_cast<int> (12.0 * sampleRate / blockSize) + 1;
    for (int block = 0; block < totalBlocks; ++block)
    {
        fillTestSignal (buffer, sampleRate, offset);
        offset += blockSize;
        for (int sample = 0; sample < blockSize; ++sample)
            inputEnergy += std::pow (buffer.getSample (0, sample), 2.0f);
        chorus.setParameters (flangerMode == 0 ? 0.32f : 1.2f, 70.0f, 90.0f,
                              8000.0f, 45.0f, true, flangerMode);
        chorus.process (buffer);
        if (block < warmupBlocks) continue;
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
    juce::ScopedJuceInitialiser_GUI juceInitialiser;
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
                if (! tremoloHarmonicVoiceIsStable (sampleRate, blockSize, channels))
                {
                    std::cerr << "FAILED Tremolo harmonic voice: " << sampleRate << " Hz, " << blockSize
                              << " samples, " << channels << " channels\n";
                    return 1;
                }
            }
        }
    if (! springAsyncLoadBoundaryIsStable())
    {
        std::cerr << "FAILED Spring asynchronous IR-load soak\n";
        return 1;
    }
    std::cout << "Rockalizer DSP validation passed\n";
    return 0;
}
