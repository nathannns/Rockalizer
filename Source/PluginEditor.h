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

    class AboutPanel final : public juce::Component
    {
    public:
        AboutPanel() : closeButton ("Close")
        {
            closeButton.onClick = [this] { setVisible (false); };
            closeButton.setTooltip ("Close About Rockalizer");
            addAndMakeVisible (closeButton);
        }
        void setLogo (juce::Image image) { logo = std::move (image); }
        void paint (juce::Graphics& g) override
        {
            g.setColour (juce::Colours::black.withAlpha (0.72f));
            g.fillAll();
            const auto card = getLocalBounds().toFloat().withSizeKeepingCentre (
                juce::jmin (560.0f, static_cast<float> (getWidth()) * 0.72f),
                juce::jmin (430.0f, static_cast<float> (getHeight()) * 0.78f));
            g.setColour (juce::Colour (0xff11161b));
            g.fillRoundedRectangle (card, 16.0f);
            g.setColour (juce::Colour (0xff495158));
            g.drawRoundedRectangle (card, 16.0f, 1.5f);
            if (logo.isValid())
                g.drawImage (logo, card.reduced (70.0f, 22.0f).removeFromTop (92.0f),
                             juce::RectanglePlacement::centred, false);
            g.setColour (juce::Colour (0xfff0eee8));
            g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
            g.drawText ("ANALOG-INSPIRED GUITAR CHARACTER SUITE",
                        juce::Rectangle<float> { card.getX() + 35.0f, card.getY() + 120.0f,
                                                 card.getWidth() - 70.0f, 28.0f },
                        juce::Justification::centred, false);
            g.setColour (juce::Colour (0xffb8bec3));
            g.setFont (juce::FontOptions (14.0f));
            g.drawFittedText (
                "Tape saturation and compression, a detuned stereo doubler, lush stereo "
                "chorus/flanger, musical echo, Fender-style tremolo, and convolution spring "
                "reverb in one focused signal path.\n\n"
                "SIGNAL FLOW\nNoise Gate  >  Low Cut  >  Doubler  >  Tape  >  Tremolo  >  "
                "Chorus/Flanger  >  Echo  >  Spring  >  Hi Cut  >  Output\n\n"
                "Rockalizer is an independent original plugin inspired by classic studio machines, "
                "guitar pedals, and the spacious records of the 1980s and 1990s.\n\nVersion 0.66.0",
                card.toNearestInt().reduced (42, 150).translated (0, 58),
                juce::Justification::centred, 12, 1.0f);
        }
        void resized() override
        {
            const auto card = getLocalBounds().toFloat().withSizeKeepingCentre (
                juce::jmin (560.0f, static_cast<float> (getWidth()) * 0.72f),
                juce::jmin (430.0f, static_cast<float> (getHeight()) * 0.78f));
            closeButton.setBounds (juce::roundToInt (card.getRight() - 48.0f),
                                   juce::roundToInt (card.getY() + 14.0f), 34, 34);
        }
    private:
        class CloseButton final : public juce::Button
        {
        public:
            explicit CloseButton (const juce::String& name) : juce::Button (name) {}
            void paintButton (juce::Graphics& g, bool highlighted, bool down) override
            {
                auto colour = juce::Colour (0xffb8bec3);
                if (highlighted) colour = juce::Colour (0xffff8a45);
                if (down) colour = colour.darker (0.18f);
                const auto area = getLocalBounds().toFloat().reduced (8.0f);
                g.setColour (colour);
                g.drawLine (juce::Line<float> { area.getTopLeft(), area.getBottomRight() }, 2.2f);
                g.drawLine (juce::Line<float> { area.getTopRight(), area.getBottomLeft() }, 2.2f);
            }
        } closeButton;
        juce::Image logo;
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

    class LedToggleButton final : public juce::Button
    {
    public:
        explicit LedToggleButton (const juce::String& name)
            : juce::Button (name), label (name.toUpperCase()) { setClickingTogglesState (true); }
        void setLedImage (const juce::Image& image) { ledImage = image; repaint(); }
        void paintButton (juce::Graphics& g, bool highlighted, bool down) override
        {
            const auto on = getToggleState();
            auto imageBounds = getLocalBounds();
            const auto ledWidth = ledImage.isValid()
                ? juce::jmin (40, juce::jmax (22, getWidth() / 2)) : 0;
            const auto imageArea = imageBounds.removeFromLeft (ledWidth).toFloat().reduced (2.0f);
            if (ledImage.isValid())
            {
                g.setOpacity (on ? 1.0f : (highlighted ? 0.34f : 0.20f));
                g.drawImage (ledImage, imageArea, juce::RectanglePlacement::centred, false);
                g.setOpacity (1.0f);
            }
            g.setColour (on ? juce::Colour (0xffffa05b)
                            : juce::Colour (0xff8b9297).withAlpha (highlighted ? 0.90f : 0.68f));
            g.setFont (juce::FontOptions (juce::jlimit (10.0f, 15.5f, static_cast<float> (getHeight()) * 0.49f),
                                          juce::Font::bold));
            g.drawText (label, ledWidth, 0, getWidth() - ledWidth, getHeight(),
                        juce::Justification::centredLeft);
            if (down)
            {
                g.setColour (juce::Colours::black.withAlpha (0.18f));
                g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);
            }
        }
    private:
        juce::Image ledImage;
        juce::String label;
    };

    class PresetIconButton final : public juce::Button
    {
    public:
        enum class Icon { add, save, remove };
        PresetIconButton (const juce::String& name, Icon iconToDraw)
            : juce::Button (name), icon (iconToDraw) {}
        void paintButton (juce::Graphics& g, bool highlighted, bool down) override
        {
            auto colour = isEnabled() ? juce::Colour (0xfff0eee8) : juce::Colour (0xff555b60);
            if (highlighted && isEnabled()) colour = juce::Colour (0xffffa05b);
            if (down) colour = colour.darker (0.18f);
            const auto b = getLocalBounds().toFloat().reduced (10.0f, 7.0f);
            g.setColour (colour);
            if (icon == Icon::add)
            {
                g.drawLine (b.getCentreX(), b.getY(), b.getCentreX(), b.getBottom(), 2.4f);
                g.drawLine (b.getX(), b.getCentreY(), b.getRight(), b.getCentreY(), 2.4f);
            }
            else if (icon == Icon::save)
            {
                g.drawRoundedRectangle (b, 2.0f, 2.0f);
                g.fillRect (b.getX() + b.getWidth() * 0.20f, b.getY(), b.getWidth() * 0.52f, b.getHeight() * 0.34f);
                g.setColour (juce::Colour (0xff151a1e));
                g.fillRect (b.getX() + b.getWidth() * 0.30f, b.getY() + 2.0f, b.getWidth() * 0.28f, b.getHeight() * 0.19f);
                g.setColour (colour);
                g.drawRoundedRectangle (b.reduced (b.getWidth() * 0.20f, b.getHeight() * 0.16f)
                                           .withTrimmedTop (b.getHeight() * 0.28f), 1.5f, 1.7f);
            }
            else
            {
                auto can = b.reduced (b.getWidth() * 0.18f, b.getHeight() * 0.12f);
                g.drawRoundedRectangle (can.withTrimmedTop (can.getHeight() * 0.22f), 1.5f, 2.0f);
                g.drawLine (can.getX() - 2.0f, can.getY() + can.getHeight() * 0.18f,
                            can.getRight() + 2.0f, can.getY() + can.getHeight() * 0.18f, 2.0f);
                g.drawLine (can.getCentreX() - 4.0f, can.getY(), can.getCentreX() + 4.0f, can.getY(), 2.0f);
            }
        }
    private:
        Icon icon;
    };

    class InvisibleLogoButton final : public juce::Button
    {
    public:
        InvisibleLogoButton() : juce::Button ("About Rockalizer")
        {
            setWantsKeyboardFocus (false);
            setMouseClickGrabsKeyboardFocus (false);
        }

        void paintButton (juce::Graphics&, bool, bool) override {}
    };

    class PluginLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        PluginLookAndFeel();
        void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPosition, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider&) override;
        juce::Font getComboBoxFont (juce::ComboBox& box) override
        {
            return juce::Font (juce::FontOptions (juce::jlimit (10.0f, 15.0f,
                                                                static_cast<float> (box.getHeight()) * 0.46f),
                                                  juce::Font::bold));
        }

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
    juce::Image flangerLedImage;
    juce::Image rockalizerLogo;
    juce::Image tapeLogo, chorusLogo, echoLogo, springLogo;
    juce::Image tapePedalImage;
    juce::Image chorusPedalImage;
    juce::Image echoPedalImage;
    juce::Image springPedalImage;

    juce::Slider inputSlider;
    juce::Slider lowCutSlider;
    juce::Slider highCutSlider;
    juce::Slider tremoloSlider;
    juce::Slider tremoloRateSlider;
    juce::TextButton tremoloBypassButton { "TREMOLO" };
    juce::Slider outputSlider;
    juce::Slider doublerSlider;
    juce::TextButton doublerBypassButton { "DOUBLER" };
    juce::Slider noiseCutSlider;
    juce::TextButton noiseGateBypassButton { "NOISE GATE" };
    juce::Slider chorusRateSlider;
    juce::Slider chorusDepthSlider;
    juce::Slider chorusWidthSlider;
    juce::Slider chorusToneSlider;
    juce::Slider chorusMixSlider;
    LedToggleButton chorusFlangerMode1Button { "I" };
    LedToggleButton chorusFlangerMode2Button { "II" };

    juce::Label chorusRateLabel;
    juce::Label chorusDepthLabel;
    juce::Label chorusWidthLabel;
    juce::Label chorusToneLabel;
    juce::Label chorusMixLabel;
    ModuleTitleButton chorusBypassButton { "CHORUS" };
    juce::Slider echoTimeSlider, echoRepeatsSlider, echoBassSlider, echoTrebleSlider;
    juce::Slider echoWobbleSlider, echoDriveSlider, echoMixSlider;
    juce::Label echoTimeLabel, echoRepeatsLabel, echoBassLabel, echoTrebleLabel;
    juce::Label echoWobbleLabel, echoDriveLabel, echoMixLabel;
    juce::ComboBox echoPatternBox;
    ModuleTitleButton echoOnButton { "ECHO" };
    LedToggleButton echoSyncButton { "SYNC" };
    juce::Slider tapeDriveSlider, tapeCompSlider, tapeToneSlider, tapeAgeSlider, tapeMixSlider, tapeVolumeSlider;
    juce::Label tapeDriveLabel, tapeCompLabel, tapeToneLabel, tapeAgeLabel, tapeMixLabel, tapeVolumeLabel;
    LedToggleButton tapeStudioButton { "STUDIO" }, tapeCassetteButton { "CASSETTE" };
    juce::ComboBox tapeOversamplingBox;
    ModuleTitleButton tapeOnButton { "TAPE" };
    juce::Slider springDecaySlider, springDwellSlider, springToneSlider, springDripSlider, springMixSlider;
    juce::Label springDecayLabel, springDwellLabel, springToneLabel, springDripLabel, springMixLabel;
    LedToggleButton spring201Button { "201" }, spring9100Button { "9100" }, springTapeMixerButton { "TAPE" };
    ModuleTitleButton springOnButton { "SPRING" };
    PowerIconButton powerButton;
    juce::TextButton presetPreviousButton { "<" };
    juce::TextButton presetNextButton { ">" };
    PresetIconButton presetSaveButton { "Save preset", PresetIconButton::Icon::save };
    PresetIconButton presetNewButton { "New preset", PresetIconButton::Icon::add };
    PresetIconButton presetDeleteButton { "Delete preset", PresetIconButton::Icon::remove };
    juce::ComboBox presetBox;
    juce::TextButton presetDropdownButton { "v" };
    OptionsPanel optionsGroup;
    AboutPanel aboutPanel;
    InvisibleLogoButton logoButton;
    GearButton optionsMenuButton;
    juce::ToggleButton input1Button { "INPUT 1" };
    juce::ToggleButton input2Button { "INPUT 2" };
    AdvancedTextButton advancedButton;
    juce::ComboBox inputLevelBox;
    juce::Label tapeOversamplingLabel;
    juce::Label tremoloVoiceLabel;
    juce::ComboBox tremoloVoiceBox;
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
    std::unique_ptr<SliderAttachment> tremoloAttachment;
    std::unique_ptr<SliderAttachment> tremoloRateAttachment;
    std::unique_ptr<ButtonAttachment> tremoloBypassAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> doublerAttachment;
    std::unique_ptr<ButtonAttachment> doublerBypassAttachment;
    std::unique_ptr<SliderAttachment> noiseCutAttachment;
    std::unique_ptr<ButtonAttachment> noiseGateBypassAttachment;
    std::unique_ptr<SliderAttachment> chorusRateAttachment;
    std::unique_ptr<SliderAttachment> chorusDepthAttachment;
    std::unique_ptr<SliderAttachment> chorusWidthAttachment;
    std::unique_ptr<SliderAttachment> chorusToneAttachment;
    std::unique_ptr<SliderAttachment> chorusMixAttachment;
    std::unique_ptr<ButtonAttachment> chorusBypassAttachment;
    std::unique_ptr<SliderAttachment> echoTimeAttachment, echoRepeatsAttachment, echoBassAttachment, echoTrebleAttachment;
    std::unique_ptr<SliderAttachment> echoWobbleAttachment, echoDriveAttachment, echoMixAttachment;
    std::unique_ptr<ComboBoxAttachment> echoPatternAttachment;
    std::unique_ptr<ButtonAttachment> echoOnAttachment, echoSyncAttachment;
    std::unique_ptr<SliderAttachment> tapeDriveAttachment, tapeCompAttachment, tapeToneAttachment;
    std::unique_ptr<SliderAttachment> tapeAgeAttachment, tapeMixAttachment, tapeVolumeAttachment;
    std::unique_ptr<ComboBoxAttachment> tapeOversamplingAttachment;
    std::unique_ptr<ButtonAttachment> tapeOnAttachment;
    std::unique_ptr<SliderAttachment> springDecayAttachment, springDwellAttachment, springToneAttachment;
    std::unique_ptr<SliderAttachment> springDripAttachment, springMixAttachment;
    std::unique_ptr<ButtonAttachment> springOnAttachment;
    std::unique_ptr<ButtonAttachment> input1Attachment;
    std::unique_ptr<ButtonAttachment> input2Attachment;
    std::unique_ptr<ComboBoxAttachment> inputLevelAttachment;
    std::unique_ptr<ComboBoxAttachment> tremoloVoiceAttachment;

    void refreshPresetList();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RockalizerAudioProcessorEditor)
};
