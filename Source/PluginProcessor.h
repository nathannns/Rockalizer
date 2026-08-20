#pragma once

#include <JuceHeader.h>
#include "DSP/ChorusModule.h"
#include "DSP/DoublerModule.h"
#include "DSP/EchoModule.h"
#include "DSP/NoiseGateModule.h"
#include "DSP/TapeModule.h"
#include "DSP/SpringModule.h"
#include "DSP/TremoloModule.h"
#include <atomic>
#include <unordered_map>

class RockalizerAudioProcessor final : public juce::AudioProcessor
{
public:
    static constexpr int factoryPresetCount = 21;
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
    bool deleteUserPreset (int presetIndex);
    int getCurrentPresetIndex() const noexcept { return currentPresetIndex; }

private:
    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;
    juce::dsp::StateVariableTPTFilter<float> lowCutFilter;
    juce::dsp::StateVariableTPTFilter<float> highCutFilter;
    ChorusModule chorusModule;
    DoublerModule doublerModule;
    EchoModule echoModule;
    NoiseGateModule noiseGateModule;
    TapeModule tapeModule;
    SpringModule springModule;
    TremoloModule tremoloModule;
    juce::AudioBuffer<float> globalDryBuffer;
    juce::SmoothedValue<float> globalWet;
    bool tapeWasActive = false;
    bool doublerWasActive = false;
    bool chorusWasActive = false;
    bool tremoloWasActive = false;
    bool echoWasActive = false;
    bool springWasActive = false;
    bool globalWasActive = true;
    double cachedTempoBpm = -1.0;
    int cachedEchoDivision = -1;
    float cachedSyncedEchoMs = 375.0f;
    int presetTransitionState = 0; // 0 idle, 1 fade out, 2 fade in
    std::atomic<bool> effectStateResetRequested { false };
    double currentSampleRate = 44100.0;
    int currentPresetIndex = 1;
    std::unordered_map<std::string, std::atomic<float>*> parameterCache;

    juce::File getUserPresetDirectory() const;
    void loadFactoryPreset (int presetIndex);
    void cacheAudioParameters();
    float readParameter (const char* parameterID) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RockalizerAudioProcessor)
};
