#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class RockalizerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit RockalizerAudioProcessorEditor (RockalizerAudioProcessor&);
    ~RockalizerAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void configureKnob (juce::Slider& slider,
                        juce::Label& label,
                        const juce::String& name,
                        const juce::String& suffix);
    void timerCallback() override;

    RockalizerAudioProcessor& processor;

    juce::Slider inputSlider;
    juce::Slider lowCutSlider;
    juce::Slider highCutSlider;
    juce::Slider outputSlider;
    juce::Slider chorusRateSlider;
    juce::Slider chorusDepthSlider;
    juce::Slider chorusWidthSlider;
    juce::Slider chorusToneSlider;
    juce::Slider chorusMixSlider;

    juce::Label chorusRateLabel;
    juce::Label chorusDepthLabel;
    juce::Label chorusWidthLabel;
    juce::Label chorusToneLabel;
    juce::Label chorusMixLabel;
    juce::ToggleButton chorusBypassButton { "ON" };
    juce::Slider echoTimeSlider, echoRepeatsSlider, echoToneSlider;
    juce::Slider echoWobbleSlider, echoDriveSlider, echoMixSlider;
    juce::Label echoTimeLabel, echoRepeatsLabel, echoToneLabel;
    juce::Label echoWobbleLabel, echoDriveLabel, echoMixLabel;
    juce::ComboBox echoPatternBox, echoDivisionBox;
    juce::ToggleButton echoOnButton { "ON" }, echoSyncButton { "SYNC" };
    juce::Slider tapeDriveSlider, tapeCompSlider, tapeToneSlider, tapeAgeSlider, tapeMixSlider;
    juce::Label tapeDriveLabel, tapeCompLabel, tapeToneLabel, tapeAgeLabel, tapeMixLabel;
    juce::ComboBox tapeTypeBox;
    juce::ToggleButton tapeOnButton { "ON" };
    juce::Slider springDecaySlider, springDwellSlider, springToneSlider, springDripSlider, springMixSlider;
    juce::Label springDecayLabel, springDwellLabel, springToneLabel, springDripLabel, springMixLabel;
    juce::ComboBox springTypeBox;
    juce::ToggleButton springOnButton { "ON" };
    juce::TextButton powerButton { "POWER" };
    float displayInputDb = -100.0f;
    float displayOutputDb = -100.0f;
    bool inputClipped = false;
    bool outputClipped = false;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> lowCutAttachment;
    std::unique_ptr<SliderAttachment> highCutAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> chorusRateAttachment;
    std::unique_ptr<SliderAttachment> chorusDepthAttachment;
    std::unique_ptr<SliderAttachment> chorusWidthAttachment;
    std::unique_ptr<SliderAttachment> chorusToneAttachment;
    std::unique_ptr<SliderAttachment> chorusMixAttachment;
    std::unique_ptr<ButtonAttachment> chorusBypassAttachment;
    std::unique_ptr<SliderAttachment> echoTimeAttachment, echoRepeatsAttachment, echoToneAttachment;
    std::unique_ptr<SliderAttachment> echoWobbleAttachment, echoDriveAttachment, echoMixAttachment;
    std::unique_ptr<ComboBoxAttachment> echoPatternAttachment, echoDivisionAttachment;
    std::unique_ptr<ButtonAttachment> echoOnAttachment, echoSyncAttachment;
    std::unique_ptr<SliderAttachment> tapeDriveAttachment, tapeCompAttachment, tapeToneAttachment;
    std::unique_ptr<SliderAttachment> tapeAgeAttachment, tapeMixAttachment;
    std::unique_ptr<ComboBoxAttachment> tapeTypeAttachment;
    std::unique_ptr<ButtonAttachment> tapeOnAttachment;
    std::unique_ptr<SliderAttachment> springDecayAttachment, springDwellAttachment, springToneAttachment;
    std::unique_ptr<SliderAttachment> springDripAttachment, springMixAttachment;
    std::unique_ptr<ComboBoxAttachment> springTypeAttachment;
    std::unique_ptr<ButtonAttachment> springOnAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RockalizerAudioProcessorEditor)
};
