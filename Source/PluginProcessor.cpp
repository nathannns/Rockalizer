#include "PluginProcessor.h"
#include "PluginEditor.h"

RockalizerAudioProcessor::RockalizerAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "ROCKALIZER_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout RockalizerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "inputGain", 1 }, "Input",
        juce::NormalisableRange<float> { -24.0f, 24.0f, 0.1f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "lowCut", 1 }, "Low Cut",
        juce::NormalisableRange<float> { 20.0f, 500.0f, 1.0f, 0.35f }, 20.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "highCut", 1 }, "Hi Cut",
        juce::NormalisableRange<float> { 2000.0f, 20000.0f, 1.0f, 0.35f }, 20000.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "outputGain", 1 }, "Output",
        juce::NormalisableRange<float> { -24.0f, 12.0f, 0.1f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "chorusOn", 1 }, "Chorus On", true));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusRate", 1 }, "Chorus Rate",
        juce::NormalisableRange<float> { 0.05f, 5.0f, 0.01f, 0.35f }, 0.6f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusDepth", 1 }, "Chorus Depth",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 35.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusWidth", 1 }, "Chorus Width",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 60.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusTone", 1 }, "Chorus Tone",
        juce::NormalisableRange<float> { 1000.0f, 16000.0f, 1.0f, 0.35f }, 8000.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusMix", 1 }, "Chorus Mix",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 25.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "echoOn", 1 }, "Echo On", true));
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "echoSync", 1 }, "Echo Sync", false));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "echoPattern", 1 }, "Echo Pattern",
        juce::StringArray { "Straight", "Bounce", "Gallop", "Cluster", "Wash" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "echoDivision", 1 }, "Echo Division",
        juce::StringArray { "1/4", "1/8", "1/8 D", "1/8 T", "1/16", "1/16 D" }, 1));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "echoTime", 1 }, "Echo Time",
        juce::NormalisableRange<float> { 40.0f, 1200.0f, 1.0f, 0.35f }, 375.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));
    for (auto item : { std::pair { "echoRepeats", "Echo Repeats" }, std::pair { "echoWobble", "Echo Wobble" },
                       std::pair { "echoDrive", "Echo Drive" }, std::pair { "echoMix", "Echo Mix" } })
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { item.first, 1 }, item.second,
            juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, juce::String (item.first) == "echoMix" ? 25.0f : 30.0f,
            juce::AudioParameterFloatAttributes().withLabel ("%")));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "echoTone", 1 }, "Echo Tone",
        juce::NormalisableRange<float> { 1200.0f, 14000.0f, 1.0f, 0.35f }, 6500.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    return layout;
}

void RockalizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    const juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32> (samplesPerBlock),
        static_cast<juce::uint32> (getTotalNumOutputChannels())
    };

    inputGain.prepare (spec);
    outputGain.prepare (spec);
    lowCutFilter.prepare (spec);
    highCutFilter.prepare (spec);
    chorusModule.prepare (spec);
    echoModule.prepare (spec);

    inputGain.setRampDurationSeconds (0.02);
    outputGain.setRampDurationSeconds (0.02);
    lowCutFilter.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    highCutFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    inputGain.reset();
    outputGain.reset();
    lowCutFilter.reset();
    highCutFilter.reset();
    chorusModule.reset();
    echoModule.reset();
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

    inputGain.setGainDecibels (parameters.getRawParameterValue ("inputGain")->load());
    lowCutFilter.setCutoffFrequency (parameters.getRawParameterValue ("lowCut")->load());
    const auto safeHighCut = juce::jmin (parameters.getRawParameterValue ("highCut")->load(),
                                         static_cast<float> (currentSampleRate * 0.45));
    highCutFilter.setCutoffFrequency (safeHighCut);
    outputGain.setGainDecibels (parameters.getRawParameterValue ("outputGain")->load());

    auto block = juce::dsp::AudioBlock<float> (buffer);
    auto context = juce::dsp::ProcessContextReplacing<float> (block);

    inputGain.process (context);
    lowCutFilter.process (context);

    chorusModule.setParameters (
        parameters.getRawParameterValue ("chorusRate")->load(),
        parameters.getRawParameterValue ("chorusDepth")->load(),
        parameters.getRawParameterValue ("chorusWidth")->load(),
        parameters.getRawParameterValue ("chorusTone")->load(),
        parameters.getRawParameterValue ("chorusMix")->load(),
        parameters.getRawParameterValue ("chorusOn")->load() > 0.5f);
    chorusModule.process (buffer);

    auto echoTime = parameters.getRawParameterValue ("echoTime")->load();
    if (parameters.getRawParameterValue ("echoSync")->load() > 0.5f)
    {
        auto bpm = 120.0;
        if (auto* playHead = getPlayHead())
            if (auto position = playHead->getPosition())
                if (auto hostBpm = position->getBpm()) bpm = *hostBpm;
        constexpr float beats[] { 1.0f, 0.5f, 0.75f, 1.0f / 3.0f, 0.25f, 0.375f };
        const auto division = juce::jlimit (0, 5, static_cast<int> (parameters.getRawParameterValue ("echoDivision")->load()));
        echoTime = static_cast<float> (60000.0 / bpm) * beats[division];
    }
    echoModule.setParameters (echoTime,
        parameters.getRawParameterValue ("echoRepeats")->load(), parameters.getRawParameterValue ("echoTone")->load(),
        parameters.getRawParameterValue ("echoWobble")->load(), parameters.getRawParameterValue ("echoDrive")->load(),
        parameters.getRawParameterValue ("echoMix")->load(), parameters.getRawParameterValue ("echoOn")->load() > 0.5f,
        static_cast<int> (parameters.getRawParameterValue ("echoPattern")->load()));
    echoModule.process (buffer);

    // Tape and Spring will be inserted around these modules later.

    highCutFilter.process (context);
    outputGain.process (context);
}

juce::AudioProcessorEditor* RockalizerAudioProcessor::createEditor()
{
    return new RockalizerAudioProcessorEditor (*this);
}

void RockalizerAudioProcessor::getStateInformation (juce::MemoryBlock& destinationData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destinationData);
}

void RockalizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RockalizerAudioProcessor();
}
