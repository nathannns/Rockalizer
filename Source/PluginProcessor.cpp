#include "PluginProcessor.h"
#include "PluginEditor.h"

RockalizerAudioProcessor::RockalizerAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "ROCKALIZER_STATE", createParameterLayout())
{
}

juce::File RockalizerAudioProcessor::getUserPresetDirectory() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
        .getChildFile ("Rockalizer").getChildFile ("Presets");
}

juce::StringArray RockalizerAudioProcessor::getPresetNames() const
{
    juce::StringArray names { "Clean Studio", "Warm Cassette", "Wide Indie", "Space Echo",
                              "Vintage Spring", "Lo-Fi Dream", "Vocal Ambience", "Guitar Room" };
    juce::Array<juce::File> files;
    getUserPresetDirectory().findChildFiles (files, juce::File::findFiles, false, "*.xml");
    juce::StringArray userNames;
    for (const auto& file : files)
        userNames.add (file.getFileNameWithoutExtension());
    userNames.sort (true);
    names.addArray (userNames);
    return names;
}

void RockalizerAudioProcessor::loadFactoryPreset (int presetIndex)
{
    for (auto* parameter : getParameters())
        parameter->setValueNotifyingHost (parameter->getDefaultValue());

    const auto set = [this] (const char* parameterID, float plainValue)
    {
        if (auto* parameter = parameters.getParameter (parameterID))
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (plainValue));
    };

    switch (presetIndex)
    {
        case 0: // Clean Studio
            set ("tapeType", 0.0f); set ("tapeDrive", 18.0f); set ("tapeComp", 20.0f);
            set ("tapeMix", 35.0f); set ("chorusMix", 10.0f); set ("echoMix", 8.0f);
            set ("springMix", 10.0f); break;
        case 1: // Warm Cassette
            set ("tapeType", 1.0f); set ("tapeDrive", 48.0f); set ("tapeComp", 55.0f);
            set ("tapeTone", 42.0f); set ("tapeAge", 45.0f); set ("tapeMix", 62.0f);
            set ("chorusMix", 12.0f); set ("echoMix", 10.0f); set ("springMix", 8.0f); break;
        case 2: // Wide Indie
            set ("tapeDrive", 24.0f); set ("tapeMix", 38.0f); set ("chorusRate", 0.45f);
            set ("chorusDepth", 46.0f); set ("chorusWidth", 88.0f); set ("chorusMix", 36.0f);
            set ("echoTime", 420.0f); set ("echoRepeats", 22.0f); set ("echoMix", 16.0f);
            set ("springType", 1.0f); set ("springMix", 18.0f); break;
        case 3: // Space Echo
            set ("tapeDrive", 32.0f); set ("echoPattern", 3.0f); set ("echoTime", 465.0f);
            set ("echoRepeats", 58.0f); set ("echoTone", 4800.0f); set ("echoWobble", 42.0f);
            set ("echoDrive", 38.0f); set ("echoMix", 38.0f); set ("springType", 2.0f);
            set ("springDecay", 48.0f); set ("springMix", 22.0f); break;
        case 4: // Vintage Spring
            set ("tapeType", 1.0f); set ("tapeDrive", 30.0f); set ("tapeMix", 42.0f);
            set ("chorusMix", 8.0f); set ("echoMix", 12.0f); set ("springType", 1.0f);
            set ("springDecay", 55.0f); set ("springDwell", 52.0f); set ("springTone", 58.0f);
            set ("springDrip", 62.0f); set ("springMix", 42.0f); break;
        case 5: // Lo-Fi Dream
            set ("tapeType", 1.0f); set ("tapeDrive", 58.0f); set ("tapeComp", 62.0f);
            set ("tapeTone", 32.0f); set ("tapeAge", 72.0f); set ("tapeMix", 70.0f);
            set ("chorusRate", 0.28f); set ("chorusDepth", 58.0f); set ("chorusMix", 34.0f);
            set ("echoPattern", 4.0f); set ("echoTime", 560.0f); set ("echoRepeats", 42.0f);
            set ("echoTone", 3300.0f); set ("echoMix", 28.0f); set ("highCut", 9800.0f);
            set ("springType", 4.0f); set ("springMix", 24.0f); break;
        case 6: // Vocal Ambience
            set ("tapeDrive", 16.0f); set ("tapeComp", 28.0f); set ("tapeMix", 28.0f);
            set ("chorusDepth", 18.0f); set ("chorusWidth", 72.0f); set ("chorusMix", 12.0f);
            set ("echoPattern", 1.0f); set ("echoTime", 310.0f); set ("echoRepeats", 18.0f);
            set ("echoMix", 14.0f); set ("springType", 0.0f); set ("springDecay", 34.0f);
            set ("springMix", 16.0f); set ("lowCut", 75.0f); break;
        case 7: // Guitar Room
            set ("tapeDrive", 28.0f); set ("tapeComp", 34.0f); set ("tapeMix", 44.0f);
            set ("chorusMix", 6.0f); set ("echoTime", 125.0f); set ("echoRepeats", 12.0f);
            set ("echoMix", 9.0f); set ("springType", 5.0f); set ("springDecay", 25.0f);
            set ("springDwell", 35.0f); set ("springMix", 22.0f); set ("lowCut", 45.0f); break;
        default: break;
    }
}

bool RockalizerAudioProcessor::loadPreset (int presetIndex)
{
    const auto names = getPresetNames();
    if (! juce::isPositiveAndBelow (presetIndex, names.size()))
        return false;

    if (presetIndex < 8)
        loadFactoryPreset (presetIndex);
    else
    {
        const auto file = getUserPresetDirectory().getChildFile (
            juce::File::createLegalFileName (names[presetIndex]) + ".xml");
        if (auto xml = juce::XmlDocument::parse (file))
        {
            if (! xml->hasTagName (parameters.state.getType()))
                return false;
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
        }
        else
            return false;
    }

    currentPresetIndex = presetIndex;
    return true;
}

bool RockalizerAudioProcessor::saveUserPreset (const juce::String& presetName)
{
    const auto legalName = juce::File::createLegalFileName (presetName.trim());
    if (legalName.isEmpty() || getUserPresetDirectory().createDirectory().failed())
        return false;

    const auto file = getUserPresetDirectory().getChildFile (legalName + ".xml");
    if (auto xml = parameters.copyState().createXml())
    {
        const auto success = file.replaceWithText (xml->toString());
        if (success)
            currentPresetIndex = getPresetNames().indexOf (legalName);
        return success;
    }
    return false;
}

juce::AudioProcessorValueTreeState::ParameterLayout RockalizerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "globalOn", 1 }, "Global On", true));

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

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "tapeOn", 1 }, "Tape On", true));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "tapeType", 1 }, "Tape Type",
        juce::StringArray { "Studio", "Cassette" }, 0));
    for (auto item : { std::pair { "tapeDrive", "Tape Drive" }, std::pair { "tapeComp", "Tape Compression" },
                       std::pair { "tapeTone", "Tape Tone" }, std::pair { "tapeAge", "Tape Age" },
                       std::pair { "tapeMix", "Tape Mix" } })
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { item.first, 1 }, item.second,
            juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, juce::String (item.first) == "tapeTone" ? 60.0f : 25.0f,
            juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "springOn", 1 }, "Spring On", true));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "springType", 1 }, "Spring Type",
        juce::StringArray { "GBSR", "Deluxe", "Space", "9100", "Echomixer", "Schaller", "Pioneer" }, 0));
    for (auto item : { std::pair { "springDecay", "Spring Decay" }, std::pair { "springDwell", "Spring Dwell" },
                       std::pair { "springTone", "Spring Tone" }, std::pair { "springDrip", "Spring Drip" },
                       std::pair { "springMix", "Spring Mix" } })
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { item.first, 1 }, item.second,
            juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, juce::String (item.first) == "springTone" ? 60.0f : 25.0f,
            juce::AudioParameterFloatAttributes().withLabel ("%")));

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
    tapeModule.prepare (spec);
    springModule.prepare (spec);

    globalDryBuffer.setSize (getTotalNumOutputChannels(), samplesPerBlock, false, false, true);
    globalWet.reset (sampleRate, 0.02);
    globalWet.setCurrentAndTargetValue (parameters.getRawParameterValue ("globalOn")->load() > 0.5f ? 1.0f : 0.0f);

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
    tapeModule.reset();
    springModule.reset();
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

    // Reject invalid host/input samples before they can enter feedback paths.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto value = buffer.getSample (channel, sample);
            if (! std::isfinite (value))
                buffer.setSample (channel, sample, 0.0f);
        }

    jassert (buffer.getNumSamples() <= globalDryBuffer.getNumSamples());
    const auto channels = juce::jmin (buffer.getNumChannels(), globalDryBuffer.getNumChannels());
    for (int channel = 0; channel < channels; ++channel)
        globalDryBuffer.copyFrom (channel, 0, buffer, channel, 0, buffer.getNumSamples());

    float inputPeak = 0.0f;
    for (int channel = 0; channel < channels; ++channel)
        inputPeak = juce::jmax (inputPeak, buffer.getMagnitude (channel, 0, buffer.getNumSamples()));
    inputPeakDb.store (juce::Decibels::gainToDecibels (inputPeak, -100.0f), std::memory_order_relaxed);
    if (inputPeak >= 1.0f)
        inputClip.store (true, std::memory_order_relaxed);
    globalWet.setTargetValue (parameters.getRawParameterValue ("globalOn")->load() > 0.5f ? 1.0f : 0.0f);

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

    tapeModule.setParameters (parameters.getRawParameterValue ("tapeDrive")->load(),
        parameters.getRawParameterValue ("tapeComp")->load(), parameters.getRawParameterValue ("tapeTone")->load(),
        parameters.getRawParameterValue ("tapeAge")->load(), parameters.getRawParameterValue ("tapeMix")->load(),
        parameters.getRawParameterValue ("tapeOn")->load() > 0.5f,
        static_cast<int> (parameters.getRawParameterValue ("tapeType")->load()));
    tapeModule.process (buffer);

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

    springModule.setParameters (parameters.getRawParameterValue ("springDecay")->load(),
        parameters.getRawParameterValue ("springDwell")->load(), parameters.getRawParameterValue ("springTone")->load(),
        parameters.getRawParameterValue ("springDrip")->load(), parameters.getRawParameterValue ("springMix")->load(),
        parameters.getRawParameterValue ("springOn")->load() > 0.5f,
        static_cast<int> (parameters.getRawParameterValue ("springType")->load()));
    springModule.process (buffer);

    highCutFilter.process (context);
    outputGain.process (context);

    // Last-resort safety rail: invalid samples become silence and runaway
    // values are bounded well above normal audio level without changing the
    // sound during ordinary operation.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto value = buffer.getSample (channel, sample);
            buffer.setSample (channel, sample,
                std::isfinite (value) ? juce::jlimit (-8.0f, 8.0f, value) : 0.0f);
        }

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto wet = globalWet.getNextValue();
        const auto dry = 1.0f - wet;
        for (int channel = 0; channel < channels; ++channel)
            buffer.setSample (channel, sample,
                buffer.getSample (channel, sample) * wet + globalDryBuffer.getSample (channel, sample) * dry);
    }

    float outputPeak = 0.0f;
    for (int channel = 0; channel < channels; ++channel)
        outputPeak = juce::jmax (outputPeak, buffer.getMagnitude (channel, 0, buffer.getNumSamples()));
    outputPeakDb.store (juce::Decibels::gainToDecibels (outputPeak, -100.0f), std::memory_order_relaxed);
    if (outputPeak >= 1.0f)
        outputClip.store (true, std::memory_order_relaxed);
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
