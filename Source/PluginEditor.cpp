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

RockalizerAudioProcessorEditor::RockalizerAudioProcessorEditor (RockalizerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
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

    chorusBypassButton.setColour (juce::ToggleButton::textColourId, primaryText);
    chorusBypassButton.setColour (juce::ToggleButton::tickColourId, accent);
    chorusBypassButton.setColour (juce::ToggleButton::tickDisabledColourId, secondaryText);
    addAndMakeVisible (chorusBypassButton);
    chorusBypassAttachment = std::make_unique<ButtonAttachment> (processor.parameters,
                                                                 "chorusOn",
                                                                 chorusBypassButton);

    setResizable (true, true);
    setResizeLimits (900, 500, 1800, 990);
    getConstrainer()->setFixedAspectRatio (static_cast<double> (referenceWidth) / referenceHeight);
    setSize (referenceWidth, referenceHeight);
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

    g.setColour (primaryText);
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText ("ROCKALIZER", 34, 20, 250, 54, juce::Justification::centredLeft);

    auto preset = juce::Rectangle<float> (310.0f, 22.0f, 560.0f, 48.0f);
    g.setColour (panel);
    g.fillRoundedRectangle (preset, 8.0f);
    g.setColour (panelBorder);
    g.drawRoundedRectangle (preset, 8.0f, 1.0f);
    g.setColour (primaryText);
    g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    g.drawText ("<       DEFAULT       >", preset, juce::Justification::centred);

    g.setColour (accent);
    g.fillEllipse (1120.0f, 28.0f, 36.0f, 36.0f);
    g.setColour (background);
    g.setFont (juce::FontOptions (19.0f, juce::Font::bold));
    g.drawText ("I", 1120, 28, 36, 36, juce::Justification::centred);

    const juce::StringArray moduleNames { "TAPE", "CHORUS", "ECHO", "SPRING" };
    const auto gap = 14.0f;
    const auto left = 28.0f;
    const auto cardsTop = 96.0f;
    const auto cardsBottom = height - 126.0f;
    const auto cardWidth = (width - left * 2.0f - gap * 3.0f) / 4.0f;

    for (int index = 0; index < moduleNames.size(); ++index)
    {
        auto card = juce::Rectangle<float> (left + index * (cardWidth + gap),
                                             cardsTop,
                                             cardWidth,
                                             cardsBottom - cardsTop);
        g.setColour (panel);
        g.fillRoundedRectangle (card, 10.0f);
        g.setColour (panelBorder);
        g.drawRoundedRectangle (card, 10.0f, 1.0f);

        g.setColour (primaryText);
        g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
        g.drawText (moduleNames[index], card.removeFromTop (62.0f), juce::Justification::centred);

        g.setColour (panelBorder);
        g.drawHorizontalLine (static_cast<int> (cardsTop + 62.0f),
                              card.getX() + 14.0f,
                              card.getRight() - 14.0f);

        if (moduleNames[index] != "CHORUS")
        {
            g.setColour (secondaryText);
            g.setFont (juce::FontOptions (14.0f));
            g.drawText ("DSP MODULE COMING NEXT", card, juce::Justification::centred);
        }
    }

    auto footer = juce::Rectangle<float> (left, height - 108.0f, width - left * 2.0f, 80.0f);
    g.setColour (panel);
    g.fillRoundedRectangle (footer, 10.0f);
    g.setColour (panelBorder);
    g.drawRoundedRectangle (footer, 10.0f, 1.0f);

    g.setColour (primaryText);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    const auto labelY = static_cast<int> (height - 102.0f);
    g.drawText ("INPUT", 42, labelY, 110, 20, juce::Justification::centred);
    g.drawText ("LOW CUT", static_cast<int> (width * 0.5f - 142.0f), labelY, 110, 20,
                juce::Justification::centred);
    g.drawText ("HI CUT", static_cast<int> (width * 0.5f + 32.0f), labelY, 110, 20,
                juce::Justification::centred);
    g.drawText ("OUTPUT", static_cast<int> (width - 152.0f), labelY, 110, 20,
                juce::Justification::centred);
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
                          juce::roundToInt (82 * scaleY));
    };

    placeChorus (chorusRateSlider, chorusRateLabel, chorusCardX + 24, 176);
    placeChorus (chorusDepthSlider, chorusDepthLabel, chorusCardX + 158, 176);
    placeChorus (chorusWidthSlider, chorusWidthLabel, chorusCardX + 24, 302);
    placeChorus (chorusToneSlider, chorusToneLabel, chorusCardX + 158, 302);
    placeChorus (chorusMixSlider, chorusMixLabel, chorusCardX + 91, 414, 100);

    chorusBypassButton.setBounds (juce::roundToInt ((chorusCardX + 205) * scaleX),
                                  juce::roundToInt (111 * scaleY),
                                  juce::roundToInt (60 * scaleX),
                                  juce::roundToInt (26 * scaleY));
}
