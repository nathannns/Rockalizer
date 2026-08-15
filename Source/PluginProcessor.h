#pragma once

#include <JuceHeader.h>
#include "DSP/ChorusModule.h"
#include "DSP/EchoModule.h"
#include "DSP/TapeModule.h"
#include "DSP/SpringModule.h"
#include <atomic>

class RockalizerAudioProcessor final : public juce::AudioProcessor
{
public:
    RockalizerAudioProcessor();
    ~RockalizerAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float> inputPeakDb { -100.0f };
    std::atomic<float> outputPeakDb { -100.0f };
    std::atomic<bool> inputClip { false };
    std::atomic<bool> outputClip { false };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::StringArray getPresetNames() const;
    bool loadPreset (int presetIndex);
    bool saveUserPreset (const juce::String& presetName);
    int getCurrentPresetIndex() const noexcept { return currentPresetIndex; }

private:
    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;
    juce::dsp::StateVariableTPTFilter<float> lowCutFilter;
    juce::dsp::StateVariableTPTFilter<float> highCutFilter;
    ChorusModule chorusModule;
    EchoModule echoModule;
    TapeModule tapeModule;
    SpringModule springModule;
    juce::AudioBuffer<float> globalDryBuffer;
    juce::SmoothedValue<float> globalWet;
    double currentSampleRate = 44100.0;
    int currentPresetIndex = 0;

    juce::File getUserPresetDirectory() const;
    void loadFactoryPreset (int presetIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RockalizerAudioProcessor)
};
