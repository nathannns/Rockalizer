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
    setResizable (true, true);
    setResizeLimits (900, 500, 1800, 990);
    setSize (referenceWidth, referenceHeight);
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

        g.setColour (secondaryText);
        g.setFont (juce::FontOptions (14.0f));
        g.drawText ("DSP MODULE COMING NEXT", card, juce::Justification::centred);
    }

    auto footer = juce::Rectangle<float> (left, height - 108.0f, width - left * 2.0f, 80.0f);
    g.setColour (panel);
    g.fillRoundedRectangle (footer, 10.0f);
    g.setColour (panelBorder);
    g.drawRoundedRectangle (footer, 10.0f, 1.0f);

    g.setColour (primaryText);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("INPUT", footer.withWidth (160.0f), juce::Justification::centred);
    g.drawText ("LOW CUT", footer.withSizeKeepingCentre (130.0f, footer.getHeight()).translated (-75.0f, 0.0f),
                juce::Justification::centred);
    g.drawText ("HI CUT", footer.withSizeKeepingCentre (130.0f, footer.getHeight()).translated (75.0f, 0.0f),
                juce::Justification::centred);
    g.drawText ("OUTPUT", footer.removeFromRight (160.0f), juce::Justification::centred);
}

void RockalizerAudioProcessorEditor::resized()
{
}
