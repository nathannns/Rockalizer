#include "PluginProcessor.h"
#include "PluginEditor.h"

RockalizerAudioProcessor::RockalizerAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

void RockalizerAudioProcessor::prepareToPlay (double, int)
{
}

void RockalizerAudioProcessor::releaseResources()
{
}

bool RockalizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();

    if (input != output)
        return false;

    return output == juce::AudioChannelSet::mono()
        || output == juce::AudioChannelSet::stereo();
}

void RockalizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto channel = getTotalNumInputChannels();
         channel < getTotalNumOutputChannels();
         ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    // V1 checkpoint: intentional transparent pass-through.
}

juce::AudioProcessorEditor* RockalizerAudioProcessor::createEditor()
{
    return new RockalizerAudioProcessorEditor (*this);
}

void RockalizerAudioProcessor::getStateInformation (juce::MemoryBlock&)
{
}

void RockalizerAudioProcessor::setStateInformation (const void*, int)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RockalizerAudioProcessor();
}
