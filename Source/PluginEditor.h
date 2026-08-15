#pragma once

#include <JuceHeader.h>
#include <BinaryData.h>
#include "PluginProcessor.h"

class RockalizerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit RockalizerAudioProcessorEditor (RockalizerAudioProcessor&);
    ~RockalizerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class OptionsPanel final : public juce::GroupComponent
    {
    public:
        OptionsPanel() : juce::GroupComponent ("optionsPanel", "OPTIONS") {}
        void paint (juce::Graphics& g) override
        {
            g.setColour (juce::Colour (0xff11161b).withAlpha (0.98f));
            g.fillRoundedRectangle (getLocalBounds().toFloat(), 9.0f);
            juce::GroupComponent::paint (g);
        }
    };

    class GearButton final : public juce::Button
    {
    public:
        GearButton() : juce::Button ("Options") {}
        void paintButton (juce::Graphics& g, bool highlighted, bool down) override
        {
            const auto bounds = getLocalBounds().toFloat().reduced (9.0f);
            const auto centre = bounds.getCentre();
            const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.27f;
            const auto colour = down ? juce::Colour (0xffff7a33)
                                     : (highlighted ? juce::Colour (0xfff0eee8)
                                                    : juce::Colour (0xffb0b5ba));
            g.setColour (colour);
            for (int tooth = 0; tooth < 8; ++tooth)
            {
                const auto angle = juce::MathConstants<float>::twoPi * static_cast<float> (tooth) / 8.0f;
                const auto inner = centre + juce::Point<float> (std::cos (angle), std::sin (angle)) * radius;
                const auto outer = centre + juce::Point<float> (std::cos (angle), std::sin (angle)) * (radius + 6.0f);
                g.drawLine ({ inner, outer }, 3.0f);
            }
            g.drawEllipse (bounds.withSizeKeepingCentre (radius * 2.2f, radius * 2.2f), 3.0f);
            g.fillEllipse (bounds.withSizeKeepingCentre (radius * 0.75f, radius * 0.75f));
        }
    };

    class ModuleTitleButton final : public juce::TextButton
    {
    public:
        explicit ModuleTitleButton (const juce::String& text) : juce::TextButton (text) {}
        void setLogo (const juce::Image& newLogo) { logo = newLogo; repaint(); }
        void paintButton (juce::Graphics& g, bool highlighted, bool) override
        {
            if (logo.isValid())
            {
                juce::Graphics::ScopedSaveState state (g);
                auto opacity = getToggleState() ? 0.92f : 0.28f;
                if (highlighted) opacity = juce::jmin (1.0f, opacity + 0.10f);
                g.setOpacity (opacity);
                const auto area = getLocalBounds().toFloat().reduced (8.0f, 2.0f);
                // All assets are normalized to their visible letters. Preserve
                // aspect ratio at one fixed letter height; longer words become
                // wider rather than shrinking their individual characters.
                auto logoHeight = juce::jmin (34.0f, area.getHeight());
                if (getButtonText() == "TAPE")
                    logoHeight *= 0.84f;
                auto logoWidth = logoHeight * static_cast<float> (logo.getWidth())
                               / static_cast<float> (logo.getHeight());
                if (logoWidth > area.getWidth())
                {
                    logoWidth = area.getWidth();
                    logoHeight = logoWidth * static_cast<float> (logo.getHeight())
                               / static_cast<float> (logo.getWidth());
                }
                const auto target = area.withSizeKeepingCentre (logoWidth, logoHeight);
                g.drawImage (logo, target, juce::RectanglePlacement::stretchToFit, false);
                return;
            }
            auto colour = findColour (getToggleState() ? juce::TextButton::textColourOnId
                                                        : juce::TextButton::textColourOffId);
            if (highlighted)
                colour = colour.brighter (0.18f);
            auto font = juce::Font (juce::FontOptions ("Futura", 27.0f, juce::Font::bold));
            font.setExtraKerningFactor (0.055f);
            g.setFont (font);
            const auto textBounds = getLocalBounds().reduced (8, 2);
            g.setColour (juce::Colours::black.withAlpha (getToggleState() ? 0.78f : 0.38f));
            g.drawFittedText (getButtonText(), textBounds.translated (2, 2),
                              juce::Justification::centred, 1);
            g.setColour (colour);
            g.drawFittedText (getButtonText(), textBounds, juce::Justification::centred, 1);
        }

    private:
        juce::Image logo;
    };

    class AdvancedTextButton final : public juce::Button
    {
    public:
        AdvancedTextButton() : juce::Button ("Advanced") { setClickingTogglesState (true); }
        void paintButton (juce::Graphics& g, bool highlighted, bool down) override
        {
            auto colour = getToggleState() ? juce::Colour (0xffff7a33)
                                           : juce::Colour (0xffb0b5ba);
            if (highlighted) colour = colour.brighter (0.14f);
            if (down) colour = colour.darker (0.12f);
            g.setColour (colour);
            g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
            g.drawFittedText ("ADVANCED", getLocalBounds().reduced (2, 1),
                              juce::Justification::centred, 1);
        }
    };

    class PowerIconButton final : public juce::TextButton
    {
    public:
        PowerIconButton() : juce::TextButton (juce::String()) { setTooltip ("Plugin on/off"); }
        void paintButton (juce::Graphics& g, bool highlighted, bool down) override
        {
            auto colour = getToggleState() ? juce::Colour (0xffff8a45)
                                           : juce::Colour (0xff70777d);
            if (highlighted) colour = colour.brighter (0.18f);
            if (down) colour = colour.darker (0.14f);

            const auto bounds = getLocalBounds().toFloat().reduced (12.0f, 8.0f);
            const auto centre = bounds.getCentre();
            const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.40f;
            juce::Path arc;
            arc.addCentredArc (centre.x, centre.y + 1.0f, radius, radius,
                               0.0f, 0.72f, juce::MathConstants<float>::twoPi - 0.72f, true);

            g.setColour (juce::Colours::black.withAlpha (0.38f));
            g.strokePath (arc, juce::PathStrokeType (3.8f));
            g.drawLine (centre.x + 2.0f, centre.y - radius - 2.0f,
                        centre.x + 2.0f, centre.y + 1.0f, 3.8f);
            g.setColour (colour);
            g.strokePath (arc, juce::PathStrokeType (2.4f,
                                                     juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
            g.drawLine (centre.x, centre.y - radius - 2.0f,
                        centre.x, centre.y + 1.0f, 2.4f);
        }
    };

    class PluginLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        PluginLookAndFeel();
        void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPosition, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider&) override;

    private:
        juce::Image knobImage;
    };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void configureKnob (juce::Slider& slider,
                        juce::Label& label,
                        const juce::String& name,
                        const juce::String& suffix);
    void timerCallback() override;
    void updateAdvancedVisibility();
    void showPresetMenu();

    RockalizerAudioProcessor& processor;
    PluginLookAndFeel pluginLookAndFeel;
    juce::Image pluginBackground;
    juce::Image rockalizerLogo;
    juce::Image tapeLogo, chorusLogo, echoLogo, springLogo;
    juce::Image tapePedalImage;
    juce::Image chorusPedalImage;
    juce::Image echoPedalImage;
    juce::Image springPedalImage;

    juce::Slider inputSlider;
    juce::Slider lowCutSlider;
    juce::Slider highCutSlider;
    juce::Slider outputSlider;
    juce::Slider noiseCutSlider;
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
    ModuleTitleButton chorusBypassButton { "CHORUS" };
    juce::Slider echoTimeSlider, echoRepeatsSlider, echoToneSlider;
    juce::Slider echoWobbleSlider, echoDriveSlider, echoMixSlider;
    juce::Label echoTimeLabel, echoRepeatsLabel, echoToneLabel;
    juce::Label echoWobbleLabel, echoDriveLabel, echoMixLabel;
    juce::ComboBox echoPatternBox;
    ModuleTitleButton echoOnButton { "ECHO" };
    juce::ToggleButton echoSyncButton { "SYNC" };
    juce::Slider tapeDriveSlider, tapeCompSlider, tapeToneSlider, tapeAgeSlider, tapeMixSlider;
    juce::Label tapeDriveLabel, tapeCompLabel, tapeToneLabel, tapeAgeLabel, tapeMixLabel;
    juce::ComboBox tapeTypeBox;
    ModuleTitleButton tapeOnButton { "TAPE" };
    juce::Slider springDecaySlider, springDwellSlider, springToneSlider, springDripSlider, springMixSlider;
    juce::Label springDecayLabel, springDwellLabel, springToneLabel, springDripLabel, springMixLabel;
    juce::ComboBox springTypeBox;
    ModuleTitleButton springOnButton { "SPRING" };
    PowerIconButton powerButton;
    juce::TextButton presetPreviousButton { "<" };
    juce::TextButton presetNextButton { ">" };
    juce::TextButton presetSaveButton { "SAVE" };
    juce::TextButton presetNewButton { "NEW" };
    juce::ComboBox presetBox;
    juce::TextButton presetDropdownButton { "v" };
    OptionsPanel optionsGroup;
    GearButton optionsMenuButton;
    juce::ToggleButton input1Button { "INPUT 1" };
    juce::ToggleButton input2Button { "INPUT 2" };
    AdvancedTextButton advancedButton;
    juce::ComboBox inputLevelBox;
    bool optionsVisible = false;
    bool advancedMode = false;
    bool lastEchoSyncState = false;
    bool snappingEchoTime = false;
    float displayInputDb = -100.0f;
    float displayOutputDb = -100.0f;
    bool inputClipped = false;
    bool outputClipped = false;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> lowCutAttachment;
    std::unique_ptr<SliderAttachment> highCutAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> noiseCutAttachment;
    std::unique_ptr<SliderAttachment> chorusRateAttachment;
    std::unique_ptr<SliderAttachment> chorusDepthAttachment;
    std::unique_ptr<SliderAttachment> chorusWidthAttachment;
    std::unique_ptr<SliderAttachment> chorusToneAttachment;
    std::unique_ptr<SliderAttachment> chorusMixAttachment;
    std::unique_ptr<ButtonAttachment> chorusBypassAttachment;
    std::unique_ptr<SliderAttachment> echoTimeAttachment, echoRepeatsAttachment, echoToneAttachment;
    std::unique_ptr<SliderAttachment> echoWobbleAttachment, echoDriveAttachment, echoMixAttachment;
    std::unique_ptr<ComboBoxAttachment> echoPatternAttachment;
    std::unique_ptr<ButtonAttachment> echoOnAttachment, echoSyncAttachment;
    std::unique_ptr<SliderAttachment> tapeDriveAttachment, tapeCompAttachment, tapeToneAttachment;
    std::unique_ptr<SliderAttachment> tapeAgeAttachment, tapeMixAttachment;
    std::unique_ptr<ComboBoxAttachment> tapeTypeAttachment;
    std::unique_ptr<ButtonAttachment> tapeOnAttachment;
    std::unique_ptr<SliderAttachment> springDecayAttachment, springDwellAttachment, springToneAttachment;
    std::unique_ptr<SliderAttachment> springDripAttachment, springMixAttachment;
    std::unique_ptr<ComboBoxAttachment> springTypeAttachment;
    std::unique_ptr<ButtonAttachment> springOnAttachment;
    std::unique_ptr<ButtonAttachment> input1Attachment;
    std::unique_ptr<ButtonAttachment> input2Attachment;
    std::unique_ptr<ComboBoxAttachment> inputLevelAttachment;

    void refreshPresetList();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RockalizerAudioProcessorEditor)
};
