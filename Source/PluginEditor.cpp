#include "PluginEditor.h"

namespace
{
constexpr int referenceWidth = 1200;
constexpr int referenceHeight = 660;

const juce::Colour background { 0xff0c1013 };
const juce::Colour panel { 0xff151a1e };
const juce::Colour panelBorder { 0xff30373c };
const juce::Colour primaryText { 0xfff0eee8 };
const juce::Colour secondaryText { 0xff92999e };
const juce::Colour accent { 0xffff7a33 };
}

RockalizerAudioProcessorEditor::PluginLookAndFeel::PluginLookAndFeel()
    : knobImage (juce::ImageFileFormat::loadFrom (BinaryData::knob_v1_png,
                                                   BinaryData::knob_v1_pngSize)
                     .rescaled (256, 256, juce::Graphics::highResamplingQuality))
{
}

void RockalizerAudioProcessorEditor::PluginLookAndFeel::drawRotarySlider (
    juce::Graphics& g, int x, int y, int width, int height, float sliderPosition,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    if (! knobImage.isValid())
        return;

    const auto diameter = static_cast<float> (juce::jmin (width, height));
    const auto left = static_cast<float> (x) + (static_cast<float> (width) - diameter) * 0.5f;
    const auto top = static_cast<float> (y) + (static_cast<float> (height) - diameter) * 0.5f;
    const auto angle = juce::jmap (sliderPosition, rotaryStartAngle, rotaryEndAngle);
    const auto scale = diameter / static_cast<float> (knobImage.getWidth());
    const auto transform = juce::AffineTransform::scale (scale)
        .followedBy (juce::AffineTransform::rotation (angle, diameter * 0.5f, diameter * 0.5f))
        .followedBy (juce::AffineTransform::translation (left, top));
    // A close contact shadow and faint mounting rim visually seat the polished
    // knob inside the textured pedal rather than floating over it.
    g.setColour (juce::Colours::black.withAlpha (0.46f));
    g.fillEllipse (left + 2.0f, top + 4.0f, diameter, diameter);
    // The source knob contains transparent antialiased pixels. Back it with an
    // opaque dark body so the pedal artwork cannot bleed through and make all
    // knobs look faded.
    g.setColour (juce::Colour (0xff101214));
    g.fillEllipse (left, top, diameter, diameter);
    // Draw the real RGBA colours. Using alpha-mask mode here turns the whole
    // asset into the current Graphics colour, which caused black silhouettes.
    g.drawImageTransformed (knobImage, transform, false);
    g.setColour (juce::Colour (0xffd6b17a).withAlpha (0.16f));
    g.drawEllipse (left + 0.5f, top + 0.5f, diameter - 1.0f, diameter - 1.0f, 1.0f);
}

RockalizerAudioProcessorEditor::RockalizerAudioProcessorEditor (RockalizerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      pluginBackground (juce::ImageFileFormat::loadFrom (BinaryData::plugin_background_png,
                                                          BinaryData::plugin_background_pngSize)
                            .rescaled (1200, 660, juce::Graphics::mediumResamplingQuality)),
      rockalizerLogo (juce::ImageFileFormat::loadFrom (BinaryData::rockalizer_logo_v1_png,
                                                        BinaryData::rockalizer_logo_v1_pngSize)),
      tapeLogo (juce::ImageFileFormat::loadFrom (BinaryData::logo_tape_v3_png,
                                                  BinaryData::logo_tape_v3_pngSize)),
      chorusLogo (juce::ImageFileFormat::loadFrom (BinaryData::logo_chorus_v2_png,
                                                    BinaryData::logo_chorus_v2_pngSize)),
      echoLogo (juce::ImageFileFormat::loadFrom (BinaryData::logo_echo_v2_png,
                                                  BinaryData::logo_echo_v2_pngSize)),
      springLogo (juce::ImageFileFormat::loadFrom (BinaryData::logo_spring_v2_png,
                                                    BinaryData::logo_spring_v2_pngSize)),
      tapePedalImage (juce::ImageFileFormat::loadFrom (BinaryData::pedal_tape_v3_png,
                                                        BinaryData::pedal_tape_v3_pngSize)),
      chorusPedalImage (juce::ImageFileFormat::loadFrom (BinaryData::pedal_chorus_v2_png,
                                                          BinaryData::pedal_chorus_v2_pngSize)),
      echoPedalImage (juce::ImageFileFormat::loadFrom (BinaryData::pedal_echo_v2_png,
                                                        BinaryData::pedal_echo_v2_pngSize)),
      springPedalImage (juce::ImageFileFormat::loadFrom (BinaryData::pedal_spring_v2_png,
                                                          BinaryData::pedal_spring_v2_pngSize))
{
    setLookAndFeel (&pluginLookAndFeel);
    tapeOnButton.setLogo (tapeLogo);
    chorusBypassButton.setLogo (chorusLogo);
    echoOnButton.setLogo (echoLogo);
    springOnButton.setLogo (springLogo);
    optionsGroup.setColour (juce::GroupComponent::outlineColourId, panelBorder);
    optionsGroup.setColour (juce::GroupComponent::textColourId, secondaryText);
    addAndMakeVisible (optionsGroup);
    optionsGroup.setVisible (false);

    optionsMenuButton.setTooltip ("Options");
    optionsMenuButton.onClick = [this]
    {
        optionsVisible = ! optionsVisible;
        optionsGroup.setVisible (optionsVisible);
        if (optionsVisible)
            optionsGroup.toFront (false);
        optionsMenuButton.toFront (false);
    };
    addAndMakeVisible (optionsMenuButton);

    for (auto* button : { &input1Button, &input2Button })
    {
        button->setColour (juce::ToggleButton::textColourId, primaryText);
        button->setColour (juce::ToggleButton::tickColourId, accent);
        button->setColour (juce::ToggleButton::tickDisabledColourId, secondaryText);
        optionsGroup.addAndMakeVisible (*button);
    }
    input1Attachment = std::make_unique<ButtonAttachment> (processor.parameters, "input1On", input1Button);
    input2Attachment = std::make_unique<ButtonAttachment> (processor.parameters, "input2On", input2Button);

    inputLevelBox.addItemList ({ "LINE", "INSTRUMENT" }, 1);
    inputLevelBox.setColour (juce::ComboBox::backgroundColourId, background);
    inputLevelBox.setColour (juce::ComboBox::textColourId, primaryText);
    inputLevelBox.setColour (juce::ComboBox::outlineColourId, panelBorder);
    optionsGroup.addAndMakeVisible (inputLevelBox);
    inputLevelAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters,
                                                                 "inputLevel",
                                                                 inputLevelBox);

    noiseCutSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    noiseCutSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    noiseCutSlider.setTooltip ("Noise Gate");
    addAndMakeVisible (noiseCutSlider);
    noiseCutAttachment = std::make_unique<SliderAttachment> (processor.parameters, "noiseCut", noiseCutSlider);

    advancedButton.onClick = [this]
    {
        advancedMode = advancedButton.getToggleState();
        updateAdvancedVisibility();
    };
    addAndMakeVisible (advancedButton);

    presetBox.setEditableText (true);
    presetBox.setJustificationType (juce::Justification::centred);
    presetBox.setColour (juce::ComboBox::backgroundColourId, panel);
    presetBox.setColour (juce::ComboBox::textColourId, primaryText);
    presetBox.setColour (juce::ComboBox::outlineColourId, panelBorder);
    addAndMakeVisible (presetBox);

    presetDropdownButton.setColour (juce::TextButton::buttonColourId, panel);
    presetDropdownButton.setColour (juce::TextButton::textColourOffId, primaryText);
    presetDropdownButton.onClick = [this] { showPresetMenu(); };
    addAndMakeVisible (presetDropdownButton);

    for (auto* button : { &presetPreviousButton, &presetNextButton, &presetNewButton, &presetSaveButton })
    {
        button->setColour (juce::TextButton::buttonColourId, panel);
        button->setColour (juce::TextButton::textColourOffId, primaryText);
        addAndMakeVisible (*button);
    }

    presetBox.onChange = [this]
    {
        if (presetBox.getSelectedItemIndex() >= 0)
            processor.loadPreset (presetBox.getSelectedItemIndex());
    };
    presetPreviousButton.onClick = [this]
    {
        const auto count = processor.getPresetNames().size();
        const auto next = (processor.getCurrentPresetIndex() - 1 + count) % count;
        if (processor.loadPreset (next)) presetBox.setSelectedItemIndex (next, juce::dontSendNotification);
    };
    presetNextButton.onClick = [this]
    {
        const auto count = processor.getPresetNames().size();
        const auto next = (processor.getCurrentPresetIndex() + 1) % count;
        if (processor.loadPreset (next)) presetBox.setSelectedItemIndex (next, juce::dontSendNotification);
    };
    presetNewButton.onClick = [this]
    {
        presetBox.setSelectedId (0, juce::dontSendNotification);
        presetBox.setText ("NEW PRESET", juce::dontSendNotification);
        presetBox.grabKeyboardFocus();
    };
    presetSaveButton.onClick = [this]
    {
        auto name = presetBox.getText().trim();
        if (name.isEmpty() || name == "NEW PRESET")
        {
            const auto names = processor.getPresetNames();
            int number = 1;
            do { name = "User Preset " + juce::String (number++); }
            while (names.contains (name));
        }
        if (processor.saveUserPreset (name)) refreshPresetList();
    };

    powerButton.setClickingTogglesState (true);
    powerButton.setColour (juce::TextButton::buttonColourId, panel);
    powerButton.setColour (juce::TextButton::buttonOnColourId, accent);
    powerButton.setColour (juce::TextButton::textColourOffId, secondaryText);
    powerButton.setColour (juce::TextButton::textColourOnId, background);
    addAndMakeVisible (powerButton);
    powerButton.setToggleState (processor.parameters.getRawParameterValue ("globalOn")->load() > 0.5f,
                                juce::dontSendNotification);
    powerButton.onClick = [this]
    {
        if (auto* parameter = processor.parameters.getParameter ("globalOn"))
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (powerButton.getToggleState() ? 1.0f : 0.0f);
            parameter->endChangeGesture();
        }
    };

    juce::Label unusedInputLabel;
    juce::Label unusedLowCutLabel;
    juce::Label unusedHighCutLabel;
    juce::Label unusedOutputLabel;
    configureKnob (inputSlider, unusedInputLabel, {}, " dB");
    configureKnob (lowCutSlider, unusedLowCutLabel, {}, " Hz");
    configureKnob (highCutSlider, unusedHighCutLabel, {}, " Hz");
    configureKnob (outputSlider, unusedOutputLabel, {}, " dB");

    configureKnob (chorusRateSlider, chorusRateLabel, "RATE", " Hz");
    configureKnob (chorusDepthSlider, chorusDepthLabel, "DEPTH", " %");
    configureKnob (chorusWidthSlider, chorusWidthLabel, "WIDTH", " %");
    configureKnob (chorusToneSlider, chorusToneLabel, "TONE", " Hz");
    configureKnob (chorusMixSlider, chorusMixLabel, "MIX", " %");

    inputAttachment = std::make_unique<SliderAttachment> (processor.parameters, "inputGain", inputSlider);
    lowCutAttachment = std::make_unique<SliderAttachment> (processor.parameters, "lowCut", lowCutSlider);
    highCutAttachment = std::make_unique<SliderAttachment> (processor.parameters, "highCut", highCutSlider);
    outputAttachment = std::make_unique<SliderAttachment> (processor.parameters, "outputGain", outputSlider);
    chorusRateAttachment = std::make_unique<SliderAttachment> (processor.parameters, "chorusRate", chorusRateSlider);
    chorusDepthAttachment = std::make_unique<SliderAttachment> (processor.parameters, "chorusDepth", chorusDepthSlider);
    chorusWidthAttachment = std::make_unique<SliderAttachment> (processor.parameters, "chorusWidth", chorusWidthSlider);
    chorusToneAttachment = std::make_unique<SliderAttachment> (processor.parameters, "chorusTone", chorusToneSlider);
    chorusMixAttachment = std::make_unique<SliderAttachment> (processor.parameters, "chorusMix", chorusMixSlider);

    chorusBypassButton.setClickingTogglesState (true);
    chorusBypassButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    chorusBypassButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    chorusBypassButton.setColour (juce::TextButton::textColourOffId, secondaryText.darker (0.55f));
    chorusBypassButton.setColour (juce::TextButton::textColourOnId, primaryText);
    addAndMakeVisible (chorusBypassButton);
    chorusBypassAttachment = std::make_unique<ButtonAttachment> (processor.parameters,
                                                                 "chorusOn",
                                                                 chorusBypassButton);

    configureKnob (echoTimeSlider, echoTimeLabel, "TIME", " ms");
    configureKnob (echoRepeatsSlider, echoRepeatsLabel, "REPEATS", " %");
    configureKnob (echoToneSlider, echoToneLabel, "TONE", " Hz");
    configureKnob (echoWobbleSlider, echoWobbleLabel, "WOBBLE", " %");
    configureKnob (echoDriveSlider, echoDriveLabel, "DRIVE", " %");
    configureKnob (echoMixSlider, echoMixLabel, "MIX", " %");
    echoTimeAttachment = std::make_unique<SliderAttachment> (processor.parameters, "echoTime", echoTimeSlider);
    echoRepeatsAttachment = std::make_unique<SliderAttachment> (processor.parameters, "echoRepeats", echoRepeatsSlider);
    echoToneAttachment = std::make_unique<SliderAttachment> (processor.parameters, "echoTone", echoToneSlider);
    echoWobbleAttachment = std::make_unique<SliderAttachment> (processor.parameters, "echoWobble", echoWobbleSlider);
    echoDriveAttachment = std::make_unique<SliderAttachment> (processor.parameters, "echoDrive", echoDriveSlider);
    echoMixAttachment = std::make_unique<SliderAttachment> (processor.parameters, "echoMix", echoMixSlider);

    echoPatternBox.addItemList ({ "STRAIGHT", "BOUNCE", "GALLOP", "CLUSTER", "WASH" }, 1);
    echoPatternBox.setColour (juce::ComboBox::backgroundColourId, panel);
    echoPatternBox.setColour (juce::ComboBox::textColourId, primaryText);
    echoPatternBox.setColour (juce::ComboBox::outlineColourId, panelBorder);
    addAndMakeVisible (echoPatternBox);
    echoPatternAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters, "echoPattern", echoPatternBox);
    echoOnButton.setClickingTogglesState (true);
    echoOnButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    echoOnButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    echoOnButton.setColour (juce::TextButton::textColourOffId, secondaryText.darker (0.55f));
    echoOnButton.setColour (juce::TextButton::textColourOnId, primaryText);
    addAndMakeVisible (echoOnButton);
    echoSyncButton.setColour (juce::ToggleButton::textColourId, primaryText);
    echoSyncButton.setColour (juce::ToggleButton::tickColourId, accent);
    addAndMakeVisible (echoSyncButton);
    echoOnAttachment = std::make_unique<ButtonAttachment> (processor.parameters, "echoOn", echoOnButton);
    echoSyncAttachment = std::make_unique<ButtonAttachment> (processor.parameters, "echoSync", echoSyncButton);
    echoTimeSlider.setTextValueSuffix ({});
    echoTimeSlider.textFromValueFunction = [this] (double value)
    {
        if (processor.parameters.getRawParameterValue ("echoSync")->load() <= 0.5f)
            return juce::String (juce::roundToInt (value)) + " ms";

        static const juce::StringArray divisions { "1/4", "1/4 D", "1/8", "1/8 D",
                                                   "1/8 T", "1/16", "1/16 D", "1/16 T" };
        const auto normalised = juce::jmap (static_cast<float> (value),
                                            static_cast<float> (echoTimeSlider.getMinimum()),
                                            static_cast<float> (echoTimeSlider.getMaximum()), 0.0f, 1.0f);
        return divisions[juce::jlimit (0, 7, juce::roundToInt (normalised * 7.0f))];
    };
    echoTimeSlider.onDragEnd = [this]
    {
        if (processor.parameters.getRawParameterValue ("echoSync")->load() <= 0.5f || snappingEchoTime)
            return;

        const auto minimum = echoTimeSlider.getMinimum();
        const auto range = echoTimeSlider.getMaximum() - minimum;
        const auto division = juce::jlimit (0, 7, juce::roundToInt (
            (echoTimeSlider.getValue() - minimum) / range * 7.0));
        if (auto* parameter = processor.parameters.getParameter ("echoDivision"))
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (static_cast<float> (division)));
            parameter->endChangeGesture();
        }
        const juce::ScopedValueSetter<bool> guard (snappingEchoTime, true);
        echoTimeSlider.setValue (minimum + range * static_cast<double> (division) / 7.0,
                                 juce::sendNotificationSync);
    };

    configureKnob (tapeDriveSlider, tapeDriveLabel, "DRIVE", " %");
    configureKnob (tapeCompSlider, tapeCompLabel, "COMP", " %");
    configureKnob (tapeToneSlider, tapeToneLabel, "TONE", " %");
    configureKnob (tapeAgeSlider, tapeAgeLabel, "AGE", " %");
    configureKnob (tapeMixSlider, tapeMixLabel, "MIX", " %");
    tapeDriveAttachment = std::make_unique<SliderAttachment> (processor.parameters, "tapeDrive", tapeDriveSlider);
    tapeCompAttachment = std::make_unique<SliderAttachment> (processor.parameters, "tapeComp", tapeCompSlider);
    tapeToneAttachment = std::make_unique<SliderAttachment> (processor.parameters, "tapeTone", tapeToneSlider);
    tapeAgeAttachment = std::make_unique<SliderAttachment> (processor.parameters, "tapeAge", tapeAgeSlider);
    tapeMixAttachment = std::make_unique<SliderAttachment> (processor.parameters, "tapeMix", tapeMixSlider);
    tapeTypeBox.addItemList ({ "STUDIO", "CASSETTE" }, 1);
    tapeTypeBox.setColour (juce::ComboBox::backgroundColourId, panel);
    tapeTypeBox.setColour (juce::ComboBox::textColourId, primaryText);
    tapeTypeBox.setColour (juce::ComboBox::outlineColourId, panelBorder);
    addAndMakeVisible (tapeTypeBox);
    tapeTypeAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters, "tapeType", tapeTypeBox);
    tapeOnButton.setClickingTogglesState (true);
    tapeOnButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    tapeOnButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    tapeOnButton.setColour (juce::TextButton::textColourOffId, secondaryText.darker (0.55f));
    tapeOnButton.setColour (juce::TextButton::textColourOnId, primaryText);
    addAndMakeVisible (tapeOnButton);
    tapeOnAttachment = std::make_unique<ButtonAttachment> (processor.parameters, "tapeOn", tapeOnButton);

    configureKnob (springDecaySlider, springDecayLabel, "DECAY", " %");
    configureKnob (springDwellSlider, springDwellLabel, "DWELL", " %");
    configureKnob (springToneSlider, springToneLabel, "TONE", " %");
    configureKnob (springDripSlider, springDripLabel, "DRIP", " %");
    configureKnob (springMixSlider, springMixLabel, "MIX", " %");
    springDecayAttachment = std::make_unique<SliderAttachment> (processor.parameters, "springDecay", springDecaySlider);
    springDwellAttachment = std::make_unique<SliderAttachment> (processor.parameters, "springDwell", springDwellSlider);
    springToneAttachment = std::make_unique<SliderAttachment> (processor.parameters, "springTone", springToneSlider);
    springDripAttachment = std::make_unique<SliderAttachment> (processor.parameters, "springDrip", springDripSlider);
    springMixAttachment = std::make_unique<SliderAttachment> (processor.parameters, "springMix", springMixSlider);
    springTypeBox.addItemList ({ "BRITISH", "DELUXE", "201", "9100", "TAPE MIXER", "GERMAN", "HI-FI" }, 1);
    springTypeBox.setColour (juce::ComboBox::backgroundColourId, panel);
    springTypeBox.setColour (juce::ComboBox::textColourId, primaryText);
    springTypeBox.setColour (juce::ComboBox::outlineColourId, panelBorder);
    addAndMakeVisible (springTypeBox);
    springTypeAttachment = std::make_unique<ComboBoxAttachment> (processor.parameters, "springType", springTypeBox);
    springOnButton.setClickingTogglesState (true);
    springOnButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    springOnButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    springOnButton.setColour (juce::TextButton::textColourOffId, secondaryText.darker (0.55f));
    springOnButton.setColour (juce::TextButton::textColourOnId, primaryText);
    addAndMakeVisible (springOnButton);
    springOnAttachment = std::make_unique<ButtonAttachment> (processor.parameters, "springOn", springOnButton);

    // Keep the header control above every subsequently-added child component.
    powerButton.toFront (false);

    setResizable (true, true);
    setResizeLimits (900, 500, 1800, 990);
    getConstrainer()->setFixedAspectRatio (static_cast<double> (referenceWidth) / referenceHeight);
    setSize (referenceWidth, referenceHeight);
    updateAdvancedVisibility();
    refreshPresetList();
    startTimerHz (30);
}

RockalizerAudioProcessorEditor::~RockalizerAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void RockalizerAudioProcessorEditor::updateAdvancedVisibility()
{
    juce::Component* advancedComponents[] {
        &tapeCompSlider, &tapeCompLabel, &tapeAgeSlider, &tapeAgeLabel,
        &chorusWidthSlider, &chorusWidthLabel, &chorusToneSlider, &chorusToneLabel,
        &echoToneSlider, &echoToneLabel, &echoWobbleSlider, &echoWobbleLabel,
        &echoDriveSlider, &echoDriveLabel, &springDwellSlider, &springDwellLabel,
        &springDripSlider, &springDripLabel
    };

    for (auto* component : advancedComponents)
        component->setVisible (advancedMode);

    resized();
    repaint();
}

void RockalizerAudioProcessorEditor::showPresetMenu()
{
    juce::PopupMenu menu;
    const auto names = processor.getPresetNames();
    for (int index = 0; index < names.size(); ++index)
        menu.addItem (index + 1, names[index], true, index == processor.getCurrentPresetIndex());

    const auto below = presetBox.localPointToGlobal (juce::Point<int> { 0, presetBox.getHeight() });
    const auto target = juce::Rectangle<int> { below.x, below.y, presetBox.getWidth(), 1 };
    const auto options = juce::PopupMenu::Options()
        .withTargetScreenArea (target)
        .withMinimumWidth (presetBox.getWidth())
        .withMaximumNumColumns (1)
        .withPreferredPopupDirection (juce::PopupMenu::Options::PopupDirection::downwards);

    menu.showMenuAsync (options,
        [safe = juce::Component::SafePointer<RockalizerAudioProcessorEditor> (this)] (int result)
        {
            if (safe != nullptr && result > 0 && safe->processor.loadPreset (result - 1))
                safe->presetBox.setSelectedItemIndex (result - 1, juce::dontSendNotification);
        });
}

void RockalizerAudioProcessorEditor::refreshPresetList()
{
    const auto names = processor.getPresetNames();
    presetBox.clear (juce::dontSendNotification);
    presetBox.addItemList (names, 1);
    presetBox.setSelectedItemIndex (juce::jlimit (0, names.size() - 1, processor.getCurrentPresetIndex()),
                                    juce::dontSendNotification);
}

void RockalizerAudioProcessorEditor::timerCallback()
{
    const auto input = processor.inputPeakDb.load (std::memory_order_relaxed);
    const auto output = processor.outputPeakDb.load (std::memory_order_relaxed);
    displayInputDb = juce::jmax (input, displayInputDb - 1.5f);
    displayOutputDb = juce::jmax (output, displayOutputDb - 1.5f);
    inputClipped = processor.inputClip.exchange (false, std::memory_order_relaxed);
    outputClipped = processor.outputClip.exchange (false, std::memory_order_relaxed);
    powerButton.setToggleState (processor.parameters.getRawParameterValue ("globalOn")->load() > 0.5f,
                                juce::dontSendNotification);

    const auto setModuleAlpha = [] (bool enabled,
                                    std::initializer_list<juce::Component*> components)
    {
        const auto alpha = enabled ? 1.0f : 0.28f;
        for (auto* component : components)
            component->setAlpha (alpha);
    };
    const auto tapeEnabled = processor.parameters.getRawParameterValue ("tapeOn")->load() > 0.5f;
    const auto chorusEnabled = processor.parameters.getRawParameterValue ("chorusOn")->load() > 0.5f;
    const auto echoEnabled = processor.parameters.getRawParameterValue ("echoOn")->load() > 0.5f;
    const auto springEnabled = processor.parameters.getRawParameterValue ("springOn")->load() > 0.5f;
    setModuleAlpha (tapeEnabled, { &tapeTypeBox, &tapeDriveSlider, &tapeToneSlider, &tapeMixSlider,
                                  &tapeCompSlider, &tapeAgeSlider, &tapeDriveLabel, &tapeToneLabel,
                                  &tapeMixLabel, &tapeCompLabel, &tapeAgeLabel });
    setModuleAlpha (chorusEnabled, { &chorusRateSlider, &chorusDepthSlider, &chorusMixSlider,
                                    &chorusWidthSlider, &chorusToneSlider, &chorusRateLabel,
                                    &chorusDepthLabel, &chorusMixLabel, &chorusWidthLabel,
                                    &chorusToneLabel });
    setModuleAlpha (echoEnabled, { &echoPatternBox, &echoSyncButton, &echoTimeSlider,
                                  &echoRepeatsSlider, &echoMixSlider, &echoToneSlider,
                                  &echoWobbleSlider, &echoDriveSlider, &echoTimeLabel,
                                  &echoRepeatsLabel, &echoMixLabel, &echoToneLabel,
                                  &echoWobbleLabel, &echoDriveLabel });
    setModuleAlpha (springEnabled, { &springTypeBox, &springDecaySlider, &springToneSlider,
                                    &springMixSlider, &springDwellSlider, &springDripSlider,
                                    &springDecayLabel, &springToneLabel, &springMixLabel,
                                    &springDwellLabel, &springDripLabel });
    const auto echoIsSynced = processor.parameters.getRawParameterValue ("echoSync")->load() > 0.5f;
    if (echoIsSynced != lastEchoSyncState)
    {
        lastEchoSyncState = echoIsSynced;
        if (echoIsSynced)
        {
            const auto division = juce::jlimit (0, 7, static_cast<int> (
                processor.parameters.getRawParameterValue ("echoDivision")->load()));
            const auto minimum = echoTimeSlider.getMinimum();
            const auto range = echoTimeSlider.getMaximum() - minimum;
            const juce::ScopedValueSetter<bool> guard (snappingEchoTime, true);
            echoTimeSlider.setValue (minimum + range * static_cast<double> (division) / 7.0,
                                     juce::dontSendNotification);
        }
        echoTimeSlider.updateText();
    }
    if (echoIsSynced && ! echoTimeSlider.isMouseButtonDown())
    {
        const auto division = juce::jlimit (0, 7, static_cast<int> (
            processor.parameters.getRawParameterValue ("echoDivision")->load()));
        const auto expected = echoTimeSlider.getMinimum()
            + (echoTimeSlider.getMaximum() - echoTimeSlider.getMinimum())
                * static_cast<double> (division) / 7.0;
        if (std::abs (echoTimeSlider.getValue() - expected) > 0.5)
        {
            const juce::ScopedValueSetter<bool> guard (snappingEchoTime, true);
            echoTimeSlider.setValue (expected, juce::dontSendNotification);
            echoTimeSlider.updateText();
        }
    }
    repaint();
}

void RockalizerAudioProcessorEditor::configureKnob (juce::Slider& slider,
                                                     juce::Label& label,
                                                     const juce::String& name,
                                                     const juce::String& suffix)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 84, 18);
    slider.setTextValueSuffix (suffix);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, panelBorder);
    slider.setColour (juce::Slider::thumbColourId, primaryText);
    slider.setColour (juce::Slider::textBoxTextColourId, primaryText);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);

    if (name.isNotEmpty())
    {
        label.setText (name, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, primaryText);
        label.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        addAndMakeVisible (label);
    }
}

void RockalizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (background);

    const auto bounds = getLocalBounds().toFloat();
    const auto scale = juce::jmin (bounds.getWidth() / static_cast<float> (referenceWidth),
                                  bounds.getHeight() / static_cast<float> (referenceHeight));

    juce::Graphics::ScopedSaveState savedState (g);
    g.addTransform (juce::AffineTransform::scale (scale));

    const auto width = bounds.getWidth() / scale;
    const auto height = bounds.getHeight() / scale;
    juce::Rectangle<float> canvas { 0.0f, 0.0f, width, height };

    if (pluginBackground.isValid())
        g.drawImage (pluginBackground, canvas, juce::RectanglePlacement::fillDestination, false);
    g.setColour (background.withAlpha (0.32f));
    g.fillRect (canvas);

    if (rockalizerLogo.isValid())
    {
        const auto logoBounds = juce::Rectangle<float> { 12.0f, 9.0f, 232.0f, 74.0f };
        g.setColour (juce::Colours::black.withAlpha (0.72f));
        g.drawImage (rockalizerLogo, logoBounds.translated (3.0f, 4.0f),
                     juce::RectanglePlacement::centred, true);
        g.drawImage (rockalizerLogo, logoBounds,
                     juce::RectanglePlacement::centred, false);
    }
    else
    {
        g.setColour (primaryText);
        g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
        g.drawText ("ROCKALIZER", 24, 20, 178, 54, juce::Justification::centredLeft);
    }
    g.setColour (secondaryText);
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText ("NOISE GATE", 260, 58, 72, 16, juce::Justification::centred);

    auto preset = juce::Rectangle<float> (342.0f, 18.0f, 530.0f, 56.0f);
    g.setColour (panel);
    g.fillRoundedRectangle (preset, 8.0f);
    g.setColour (panelBorder);
    g.drawRoundedRectangle (preset, 8.0f, 1.0f);
    const juce::StringArray moduleNames { "TAPE", "CHORUS", "ECHO", "SPRING" };
    const auto gap = 14.0f;
    const auto left = 28.0f;
    const auto cardsTop = 96.0f;
    const auto cardsBottom = height - 126.0f;
    const auto cardWidth = (width - left * 2.0f - gap * 3.0f) / 4.0f;

    const juce::Image* pedalImages[] {
        &tapePedalImage, &chorusPedalImage, &echoPedalImage, &springPedalImage
    };
    const juce::Rectangle<float> pedalSourceAreas[] {
        { 58.0f, 0.0f, 908.0f, 1536.0f },
        { 58.0f, 0.0f, 908.0f, 1536.0f },
        { 74.0f, 0.0f, 876.0f, 1536.0f },
        { 58.0f, 0.0f, 908.0f, 1536.0f }
    };
    const bool moduleEnabled[] {
        processor.parameters.getRawParameterValue ("tapeOn")->load() > 0.5f,
        processor.parameters.getRawParameterValue ("chorusOn")->load() > 0.5f,
        processor.parameters.getRawParameterValue ("echoOn")->load() > 0.5f,
        processor.parameters.getRawParameterValue ("springOn")->load() > 0.5f
    };

    for (int index = 0; index < moduleNames.size(); ++index)
    {
        auto card = juce::Rectangle<float> (left + static_cast<float> (index) * (cardWidth + gap),
                                             cardsTop,
                                             cardWidth,
                                             cardsBottom - cardsTop);
        const auto face = card;
        if (pedalImages[index]->isValid())
        {
            juce::Graphics::ScopedSaveState imageState (g);
            g.setOpacity (moduleEnabled[index] ? 1.0f : 0.28f);
            juce::Path clip;
            clip.addRoundedRectangle (face, 12.0f);
            g.reduceClipRegion (clip);
            const auto source = pedalSourceAreas[index];
            g.drawImage (*pedalImages[index],
                         juce::roundToInt (face.getX()),
                         juce::roundToInt (face.getY()),
                         juce::roundToInt (face.getWidth()),
                         juce::roundToInt (face.getHeight()),
                         juce::roundToInt (source.getX()),
                         juce::roundToInt (source.getY()),
                         juce::roundToInt (source.getWidth()),
                         juce::roundToInt (source.getHeight()),
                         false);
        }
        card.removeFromTop (62.0f);

        if (moduleNames[index] != "TAPE" && moduleNames[index] != "CHORUS"
            && moduleNames[index] != "ECHO" && moduleNames[index] != "SPRING")
        {
            g.setColour (secondaryText);
            g.setFont (juce::FontOptions (14.0f));
            g.drawText ("DSP MODULE COMING NEXT", card, juce::Justification::centred);
        }
    }

    auto footer = juce::Rectangle<float> (left, height - 108.0f, width - left * 2.0f, 100.0f);
    g.setColour (panel.withAlpha (0.72f));
    g.fillRoundedRectangle (footer, 10.0f);
    g.setColour (panelBorder.withAlpha (0.66f));
    g.drawRoundedRectangle (footer, 10.0f, 1.0f);

    g.setColour (primaryText);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    const auto labelY = static_cast<int> (height - 102.0f);
    const auto instrumentInput = processor.parameters.getRawParameterValue ("inputLevel")->load() > 0.5f;
    g.drawText (instrumentInput ? "GUITAR IN" : "LINE IN", 42, labelY, 110, 20,
                juce::Justification::centred);
    g.drawText ("LOW CUT", static_cast<int> (width * 0.5f - 142.0f), labelY, 110, 20,
                juce::Justification::centred);
    g.drawText ("HI CUT", static_cast<int> (width * 0.5f + 32.0f), labelY, 110, 20,
                juce::Justification::centred);
    g.drawText ("OUTPUT", static_cast<int> (width - 152.0f), labelY, 110, 20,
                juce::Justification::centred);

    const auto drawMeter = [&g] (juce::Rectangle<float> meter, float levelDb, bool clipped)
    {
        g.setColour (background);
        g.fillRoundedRectangle (meter, 4.0f);
        const auto proportion = juce::jlimit (0.0f, 1.0f, juce::jmap (levelDb, -60.0f, 0.0f, 0.0f, 1.0f));
        auto fill = meter.withWidth (meter.getWidth() * proportion);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff38c878), meter.getX(), meter.getY(),
                                                 clipped ? juce::Colour (0xffff4545) : juce::Colour (0xffffb13b),
                                                 meter.getRight(), meter.getY(), false));
        g.fillRoundedRectangle (fill, 4.0f);
        g.setColour (clipped ? juce::Colour (0xffff4545) : panelBorder);
        g.drawRoundedRectangle (meter, 4.0f, clipped ? 2.0f : 1.0f);
    };
    drawMeter ({ 154.0f, height - 67.0f, 248.0f, 12.0f }, displayInputDb, inputClipped);
    drawMeter ({ width - 402.0f, height - 67.0f, 248.0f, 12.0f }, displayOutputDb, outputClipped);
}

void RockalizerAudioProcessorEditor::resized()
{
    const auto scaleX = getWidth() / static_cast<float> (referenceWidth);
    const auto scaleY = getHeight() / static_cast<float> (referenceHeight);

    const auto place = [scaleX, scaleY] (juce::Slider& slider, int x, int y)
    {
        slider.setBounds (juce::roundToInt (x * scaleX),
                          juce::roundToInt (y * scaleY),
                          juce::roundToInt (110 * scaleX),
                          juce::roundToInt (78 * scaleY));
    };

    place (inputSlider, 42, 570);
    place (lowCutSlider, 458, 570);
    place (highCutSlider, 632, 570);
    place (outputSlider, 1048, 570);
    noiseCutSlider.setBounds (juce::roundToInt (270 * scaleX), juce::roundToInt (18 * scaleY),
                              juce::roundToInt (52 * scaleX), juce::roundToInt (40 * scaleY));
    optionsGroup.setBounds (juce::roundToInt (898 * scaleX), juce::roundToInt (82 * scaleY),
                            juce::roundToInt (282 * scaleX), juce::roundToInt (88 * scaleY));
    input1Button.setBounds (juce::roundToInt (12 * scaleX), juce::roundToInt (30 * scaleY),
                            juce::roundToInt (72 * scaleX), juce::roundToInt (32 * scaleY));
    input2Button.setBounds (juce::roundToInt (88 * scaleX), juce::roundToInt (30 * scaleY),
                            juce::roundToInt (72 * scaleX), juce::roundToInt (32 * scaleY));
    inputLevelBox.setBounds (juce::roundToInt (164 * scaleX), juce::roundToInt (30 * scaleY),
                             juce::roundToInt (106 * scaleX), juce::roundToInt (32 * scaleY));
    advancedButton.setBounds (juce::roundToInt (892 * scaleX), juce::roundToInt (18 * scaleY),
                              juce::roundToInt (92 * scaleX), juce::roundToInt (56 * scaleY));
    optionsMenuButton.setBounds (juce::roundToInt (1004 * scaleX), juce::roundToInt (18 * scaleY),
                                 juce::roundToInt (56 * scaleX), juce::roundToInt (56 * scaleY));
    optionsMenuButton.toFront (false);
    advancedButton.toFront (false);
    powerButton.setBounds (juce::roundToInt (1080 * scaleX), juce::roundToInt (18 * scaleY),
                           juce::roundToInt (60 * scaleX), juce::roundToInt (56 * scaleY));
    presetPreviousButton.setBounds (juce::roundToInt (352 * scaleX), juce::roundToInt (28 * scaleY),
                                    juce::roundToInt (38 * scaleX), juce::roundToInt (36 * scaleY));
    presetBox.setBounds (juce::roundToInt (398 * scaleX), juce::roundToInt (28 * scaleY),
                         juce::roundToInt (256 * scaleX), juce::roundToInt (36 * scaleY));
    presetDropdownButton.setBounds (juce::roundToInt (618 * scaleX), juce::roundToInt (28 * scaleY),
                                    juce::roundToInt (36 * scaleX), juce::roundToInt (36 * scaleY));
    presetDropdownButton.toFront (false);
    presetNextButton.setBounds (juce::roundToInt (662 * scaleX), juce::roundToInt (28 * scaleY),
                                juce::roundToInt (38 * scaleX), juce::roundToInt (36 * scaleY));
    presetNewButton.setBounds (juce::roundToInt (708 * scaleX), juce::roundToInt (28 * scaleY),
                               juce::roundToInt (68 * scaleX), juce::roundToInt (36 * scaleY));
    presetSaveButton.setBounds (juce::roundToInt (784 * scaleX), juce::roundToInt (28 * scaleY),
                                juce::roundToInt (74 * scaleX), juce::roundToInt (36 * scaleY));

    const auto chorusCardX = 318;
    const auto placeChorus = [scaleX, scaleY] (juce::Slider& slider,
                                               juce::Label& label,
                                               int x,
                                               int y,
                                               int size = 90)
    {
        label.setBounds (juce::roundToInt (x * scaleX),
                         juce::roundToInt (y * scaleY),
                         juce::roundToInt (size * scaleX),
                         juce::roundToInt (18 * scaleY));
        slider.setBounds (juce::roundToInt (x * scaleX),
                          juce::roundToInt ((y + 18) * scaleY),
                          juce::roundToInt (size * scaleX),
                          juce::roundToInt (74 * scaleY));
    };

    placeChorus (chorusRateSlider, chorusRateLabel, chorusCardX + 24, 150, 68);
    placeChorus (chorusDepthSlider, chorusDepthLabel, chorusCardX + 104, 150, 68);
    placeChorus (chorusMixSlider, chorusMixLabel, chorusCardX + 184, 150, 68);
    placeChorus (chorusWidthSlider, chorusWidthLabel, chorusCardX + 66, 275, 68);
    placeChorus (chorusToneSlider, chorusToneLabel, chorusCardX + 142, 275, 68);

    chorusBypassButton.setBounds (juce::roundToInt ((chorusCardX + 18) * scaleX),
                                  juce::roundToInt (438 * scaleY),
                                  juce::roundToInt (240 * scaleX),
                                  juce::roundToInt (46 * scaleY));

    const auto echoCardX = 608;
    const auto placeEcho = [scaleX, scaleY] (juce::Slider& slider, juce::Label& label,
                                             int x, int y, int size = 90)
    {
        label.setBounds (juce::roundToInt (x * scaleX), juce::roundToInt (y * scaleY),
                         juce::roundToInt (size * scaleX), juce::roundToInt (18 * scaleY));
        slider.setBounds (juce::roundToInt (x * scaleX), juce::roundToInt ((y + 18) * scaleY),
                          juce::roundToInt (size * scaleX), juce::roundToInt (74 * scaleY));
    };
    echoPatternBox.setBounds (juce::roundToInt ((echoCardX + 42) * scaleX), juce::roundToInt (402 * scaleY),
                              juce::roundToInt (120 * scaleX), juce::roundToInt (28 * scaleY));
    echoSyncButton.setBounds (juce::roundToInt ((echoCardX + 166) * scaleX), juce::roundToInt (402 * scaleY),
                              juce::roundToInt (68 * scaleX), juce::roundToInt (28 * scaleY));
    placeEcho (echoTimeSlider, echoTimeLabel, echoCardX + 24, 150, 68);
    placeEcho (echoRepeatsSlider, echoRepeatsLabel, echoCardX + 104, 150, 68);
    placeEcho (echoMixSlider, echoMixLabel, echoCardX + 184, 150, 68);
    placeEcho (echoToneSlider, echoToneLabel, echoCardX + 24, 275, 68);
    placeEcho (echoWobbleSlider, echoWobbleLabel, echoCardX + 104, 275, 68);
    placeEcho (echoDriveSlider, echoDriveLabel, echoCardX + 184, 275, 68);
    echoOnButton.setBounds (juce::roundToInt ((echoCardX + 18) * scaleX), juce::roundToInt (438 * scaleY),
                            juce::roundToInt (240 * scaleX), juce::roundToInt (46 * scaleY));

    const auto tapeCardX = 28;
    tapeTypeBox.setBounds (juce::roundToInt ((tapeCardX + 42) * scaleX), juce::roundToInt (402 * scaleY),
                           juce::roundToInt (192 * scaleX), juce::roundToInt (28 * scaleY));
    const auto placeTape = [scaleX, scaleY] (juce::Slider& slider, juce::Label& label,
                                             int x, int y, int size = 90)
    {
        label.setBounds (juce::roundToInt (x * scaleX), juce::roundToInt (y * scaleY),
                         juce::roundToInt (size * scaleX), juce::roundToInt (18 * scaleY));
        slider.setBounds (juce::roundToInt (x * scaleX), juce::roundToInt ((y + 18) * scaleY),
                          juce::roundToInt (size * scaleX), juce::roundToInt (74 * scaleY));
    };
    placeTape (tapeDriveSlider, tapeDriveLabel, tapeCardX + 24, 150, 68);
    placeTape (tapeToneSlider, tapeToneLabel, tapeCardX + 104, 150, 68);
    placeTape (tapeMixSlider, tapeMixLabel, tapeCardX + 184, 150, 68);
    placeTape (tapeCompSlider, tapeCompLabel, tapeCardX + 66, 275, 68);
    placeTape (tapeAgeSlider, tapeAgeLabel, tapeCardX + 142, 275, 68);
    tapeOnButton.setBounds (juce::roundToInt ((tapeCardX + 18) * scaleX), juce::roundToInt (438 * scaleY),
                            juce::roundToInt (240 * scaleX), juce::roundToInt (46 * scaleY));

    const auto springCardX = 898;
    springTypeBox.setBounds (juce::roundToInt ((springCardX + 42) * scaleX), juce::roundToInt (402 * scaleY),
                             juce::roundToInt (192 * scaleX), juce::roundToInt (28 * scaleY));
    const auto placeSpring = [scaleX, scaleY] (juce::Slider& slider, juce::Label& label,
                                               int x, int y, int size = 90)
    {
        label.setBounds (juce::roundToInt (x * scaleX), juce::roundToInt (y * scaleY),
                         juce::roundToInt (size * scaleX), juce::roundToInt (18 * scaleY));
        slider.setBounds (juce::roundToInt (x * scaleX), juce::roundToInt ((y + 18) * scaleY),
                          juce::roundToInt (size * scaleX), juce::roundToInt (74 * scaleY));
    };
    placeSpring (springDecaySlider, springDecayLabel, springCardX + 24, 150, 68);
    placeSpring (springToneSlider, springToneLabel, springCardX + 104, 150, 68);
    placeSpring (springMixSlider, springMixLabel, springCardX + 184, 150, 68);
    placeSpring (springDwellSlider, springDwellLabel, springCardX + 66, 275, 68);
    placeSpring (springDripSlider, springDripLabel, springCardX + 142, 275, 68);
    springOnButton.setBounds (juce::roundToInt ((springCardX + 18) * scaleX), juce::roundToInt (438 * scaleY),
                              juce::roundToInt (240 * scaleX), juce::roundToInt (46 * scaleY));
}
