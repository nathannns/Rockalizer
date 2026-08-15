#include "PluginProcessor.h"
#include "PluginEditor.h"

RockalizerAudioProcessor::RockalizerAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "ROCKALIZER_STATE", createParameterLayout())
{
    // The header initially names Clean Studio, so a fresh instance must start
    // with that preset's values rather than unrelated parameter defaults.
    loadPreset (0);
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
    {
        const auto* identified = dynamic_cast<juce::AudioProcessorParameterWithID*> (parameter);
        if (identified == nullptr
            || (identified->paramID != "input1On" && identified->paramID != "input2On"
                && identified->paramID != "noiseCut" && identified->paramID != "inputLevel"))
            parameter->setValueNotifyingHost (parameter->getDefaultValue());
    }

    const auto set = [this] (const char* parameterID, float plainValue)
    {
        if (auto* parameter = parameters.getParameter (parameterID))
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (plainValue));
    };

    switch (presetIndex)
    {
        case 0: // Clean Studio
            set ("tapeType", 0.0f); set ("tapeDrive", 12.0f); set ("tapeComp", 20.0f);
            set ("tapeMix", 35.0f); set ("chorusRate", 0.30f); set ("chorusDepth", 30.0f);
            set ("chorusWidth", 76.0f); set ("chorusMix", 12.0f); set ("echoMix", 8.0f);
            set ("springMix", 10.0f); break;
        case 1: // Warm Cassette
            set ("tapeType", 1.0f); set ("tapeDrive", 34.0f); set ("tapeComp", 48.0f);
            set ("tapeTone", 42.0f); set ("tapeAge", 45.0f); set ("tapeMix", 62.0f);
            set ("chorusRate", 0.24f); set ("chorusDepth", 34.0f); set ("chorusWidth", 78.0f);
            set ("chorusMix", 15.0f); set ("echoMix", 10.0f); set ("springMix", 8.0f); break;
        case 2: // Wide Indie
            set ("tapeDrive", 17.0f); set ("tapeMix", 38.0f); set ("chorusRate", 0.38f);
            set ("chorusDepth", 58.0f); set ("chorusWidth", 94.0f); set ("chorusMix", 40.0f);
            set ("echoTime", 420.0f); set ("echoRepeats", 22.0f); set ("echoMix", 16.0f);
            set ("springType", 1.0f); set ("springMix", 18.0f); break;
        case 3: // Space Echo
            set ("tapeDrive", 22.0f); set ("chorusRate", 0.22f); set ("chorusDepth", 26.0f);
            set ("chorusWidth", 74.0f); set ("chorusMix", 10.0f);
            set ("echoPattern", 3.0f); set ("echoTime", 465.0f);
            set ("echoRepeats", 36.0f); set ("echoTone", 4800.0f); set ("echoWobble", 38.0f);
            set ("echoDrive", 20.0f); set ("echoMix", 36.0f); set ("springType", 2.0f);
            set ("springDecay", 48.0f); set ("springMix", 22.0f); break;
        case 4: // Vintage Spring
            set ("tapeType", 1.0f); set ("tapeDrive", 21.0f); set ("tapeMix", 42.0f);
            set ("chorusRate", 0.20f); set ("chorusDepth", 24.0f); set ("chorusWidth", 70.0f);
            set ("chorusMix", 9.0f); set ("echoMix", 12.0f); set ("springType", 1.0f);
            set ("springDecay", 55.0f); set ("springDwell", 52.0f); set ("springTone", 58.0f);
            set ("springDrip", 62.0f); set ("springMix", 42.0f); break;
        case 5: // Lo-Fi Dream
            set ("tapeType", 1.0f); set ("tapeDrive", 42.0f); set ("tapeComp", 56.0f);
            set ("tapeTone", 32.0f); set ("tapeAge", 72.0f); set ("tapeMix", 70.0f);
            set ("chorusRate", 0.26f); set ("chorusDepth", 64.0f); set ("chorusWidth", 92.0f);
            set ("chorusMix", 38.0f);
            set ("echoPattern", 4.0f); set ("echoTime", 560.0f); set ("echoRepeats", 36.0f);
            set ("echoTone", 3300.0f); set ("echoMix", 28.0f); set ("highCut", 9800.0f);
            set ("springType", 4.0f); set ("springMix", 24.0f); break;
        case 6: // Vocal Ambience
            set ("tapeDrive", 11.0f); set ("tapeComp", 28.0f); set ("tapeMix", 28.0f);
            set ("chorusRate", 0.24f); set ("chorusDepth", 28.0f);
            set ("chorusWidth", 80.0f); set ("chorusMix", 14.0f);
            set ("echoPattern", 1.0f); set ("echoTime", 310.0f); set ("echoRepeats", 18.0f);
            set ("echoMix", 14.0f); set ("springType", 0.0f); set ("springDecay", 34.0f);
            set ("springMix", 16.0f); set ("lowCut", 75.0f); break;
        case 7: // Guitar Room
            set ("tapeDrive", 19.0f); set ("tapeComp", 34.0f); set ("tapeMix", 44.0f);
            set ("chorusRate", 0.18f); set ("chorusDepth", 20.0f);
            set ("chorusWidth", 68.0f); set ("chorusMix", 8.0f);
            set ("echoTime", 125.0f); set ("echoRepeats", 12.0f);
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

    // Input connector selection belongs to the user's interface setup, not to
    // the sound preset. Keep it unchanged while factory or user presets load.
    const auto input1WasOn = parameters.getRawParameterValue ("input1On")->load() > 0.5f;
    const auto input2WasOn = parameters.getRawParameterValue ("input2On")->load() > 0.5f;
    const auto noiseCutWas = parameters.getRawParameterValue ("noiseCut")->load();
    const auto inputLevelWas = parameters.getRawParameterValue ("inputLevel")->load();

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

    const auto restoreSwitch = [this] (const char* parameterID, bool enabled)
    {
        if (auto* parameter = parameters.getParameter (parameterID))
            parameter->setValueNotifyingHost (enabled ? 1.0f : 0.0f);
    };
    restoreSwitch ("input1On", input1WasOn);
    restoreSwitch ("input2On", input2WasOn);
    if (auto* parameter = parameters.getParameter ("noiseCut"))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (noiseCutWas));
    if (auto* parameter = parameters.getParameter ("inputLevel"))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (inputLevelWas));

    currentPresetIndex = presetIndex;
    // Loading several time-based parameters at once must not pitch-sweep the
    // previous preset's delay/reverb memory. The audio thread performs this
    // reset safely at its next block boundary; each module then fades its new
    // wet signal in using its existing SmoothedValue.
    effectStateResetRequested.store (true, std::memory_order_release);
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

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "input1On", 1 }, "Input 1", false));
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "input2On", 1 }, "Input 2", true));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "inputLevel", 1 }, "Input Level",
        juce::StringArray { "Line", "Instrument" }, 1));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "inputGain", 1 }, "Input",
        juce::NormalisableRange<float> { -24.0f, 24.0f, 0.1f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "noiseCut", 1 }, "Noise Gate",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

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
        juce::NormalisableRange<float> { 0.05f, 5.0f, 0.01f, 0.35f }, 0.32f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusDepth", 1 }, "Chorus Depth",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 42.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusWidth", 1 }, "Chorus Width",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 80.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusTone", 1 }, "Chorus Tone",
        juce::NormalisableRange<float> { 1000.0f, 16000.0f, 1.0f, 0.35f }, 8000.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusMix", 1 }, "Chorus Mix",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 20.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "echoOn", 1 }, "Echo On", true));
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "echoSync", 1 }, "Echo Sync", false));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "echoPattern", 1 }, "Echo Pattern",
        juce::StringArray { "Straight", "Bounce", "Gallop", "Cluster", "Wash" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "echoDivision", 1 }, "Echo Division",
        juce::StringArray { "1/4", "1/4 D", "1/8", "1/8 D", "1/8 T", "1/16", "1/16 D", "1/16 T" }, 2));
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
    {
        auto defaultValue = 25.0f;
        if (juce::String (item.first) == "tapeTone") defaultValue = 60.0f;
        if (juce::String (item.first) == "tapeComp") defaultValue = 20.0f;
        if (juce::String (item.first) == "tapeAge") defaultValue = 0.0f;
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { item.first, 1 }, item.second,
            juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, defaultValue,
            juce::AudioParameterFloatAttributes().withLabel ("%")));
    }

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "springOn", 1 }, "Spring On", true));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "springType", 1 }, "Spring Type",
        juce::StringArray { "British", "Deluxe", "201", "9100", "Tape Mixer", "German", "Hi-Fi" }, 0));
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
    noiseGateBandEnvelope.fill (0.0f);
    noiseGateLowState = noiseGateMidState = 0.0f;
    noiseGateGain = 1.0f;
    noiseGateHoldSamples = 0;
    noiseGateOpen = true;

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

    if (effectStateResetRequested.exchange (false, std::memory_order_acq_rel))
    {
        tapeModule.reset();
        chorusModule.reset();
        echoModule.reset();
        springModule.reset();
    }

    for (auto channel = getTotalNumInputChannels();
         channel < getTotalNumOutputChannels();
         ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    // Audio interfaces expose Input 1 as left and Input 2 as right. Sum the
    // enabled connectors to mono, average when both are enabled, and explicitly
    // write the result to both channels so output can never be right-only.
    if (buffer.getNumChannels() >= 2)
    {
        const auto useInput1 = parameters.getRawParameterValue ("input1On")->load() > 0.5f;
        const auto useInput2 = parameters.getRawParameterValue ("input2On")->load() > 0.5f;
        auto* left = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);
        const auto gain = useInput1 && useInput2 ? 0.5f : 1.0f;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto mono = ((useInput1 ? left[sample] : 0.0f)
                             + (useInput2 ? right[sample] : 0.0f)) * gain;
            left[sample] = mono;
            right[sample] = mono;
        }
    }
    else if (buffer.getNumChannels() == 1)
    {
        // A mono host bus only exposes connector 1. Respect the switches:
        // Input 2 cannot be selected when the host has not supplied it.
        const auto useInput1 = parameters.getRawParameterValue ("input1On")->load() > 0.5f;
        if (! useInput1)
            buffer.clear();
    }

    // Digital calibration after the interface conversion. Instrument mode
    // raises guitar-level signals by 6 dB; Line mode preserves full headroom.
    if (parameters.getRawParameterValue ("inputLevel")->load() > 0.5f)
        buffer.applyGain (juce::Decibels::decibelsToGain (6.0f));

    // Guitar-focused, stereo-linked downward expander. Three frequency-trimmed
    // detector bands keep fundamentals, pick attack and upper harmonics equally
    // capable of opening the gate. Hysteresis and hold prevent chatter, while
    // the audio path itself remains full-band and phase-transparent.
    const auto noiseCut = parameters.getRawParameterValue ("noiseCut")->load() * 0.01f;
    if (noiseCut > 0.0001f)
    {
        const auto openThresholdDb = juce::jmap (noiseCut, -78.0f, -32.0f);
        const auto closeThresholdDb = openThresholdDb - 6.0f;
        const auto detectorAttack = std::exp (-1.0f / static_cast<float> (currentSampleRate * 0.0012));
        const auto detectorRelease = std::exp (-1.0f / static_cast<float> (currentSampleRate * 0.085));
        const auto gateAttack = std::exp (-1.0f / static_cast<float> (currentSampleRate * 0.0008));
        const auto gateRelease = std::exp (-1.0f / static_cast<float> (currentSampleRate * 0.180));
        const auto lowCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * 180.0f
                                                     / static_cast<float> (currentSampleRate));
        const auto midCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * 2600.0f
                                                     / static_cast<float> (currentSampleRate));
        const auto holdLength = juce::roundToInt (currentSampleRate * 0.045);
        const std::array<float, 3> bandOffsetsDb { 2.0f, 0.0f, -2.0f };

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float detector = 0.0f;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                const auto candidate = buffer.getSample (channel, sample);
                if (std::abs (candidate) > std::abs (detector))
                    detector = candidate;
            }

            noiseGateLowState += lowCoefficient * (detector - noiseGateLowState);
            noiseGateMidState += midCoefficient * (detector - noiseGateMidState);
            const std::array<float, 3> bands {
                std::abs (noiseGateLowState),
                std::abs (noiseGateMidState - noiseGateLowState),
                std::abs (detector - noiseGateMidState)
            };

            float activityDb = -120.0f;
            for (size_t band = 0; band < bands.size(); ++band)
            {
                auto& envelope = noiseGateBandEnvelope[band];
                const auto coefficient = bands[band] > envelope ? detectorAttack : detectorRelease;
                envelope = coefficient * envelope + (1.0f - coefficient) * bands[band];
                activityDb = juce::jmax (activityDb,
                    juce::Decibels::gainToDecibels (envelope, -120.0f) - bandOffsetsDb[band]);
            }

            if (activityDb >= openThresholdDb)
            {
                noiseGateOpen = true;
                noiseGateHoldSamples = holdLength;
            }
            else if (noiseGateHoldSamples > 0)
                --noiseGateHoldSamples;
            else if (activityDb < closeThresholdDb)
                noiseGateOpen = false;

            const auto floorDb = juce::jmap (noiseCut, -18.0f, -72.0f);
            const auto belowDb = juce::jmax (0.0f, closeThresholdDb - activityDb);
            const auto closedGainDb = juce::jmax (floorDb, -belowDb * 2.2f);
            const auto targetGain = noiseGateOpen ? 1.0f
                                                  : juce::Decibels::decibelsToGain (closedGainDb);
            const auto gainCoefficient = targetGain > noiseGateGain ? gateAttack : gateRelease;
            noiseGateGain = gainCoefficient * noiseGateGain + (1.0f - gainCoefficient) * targetGain;

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.setSample (channel, sample, buffer.getSample (channel, sample) * noiseGateGain);
        }
    }
    else
    {
        noiseGateBandEnvelope.fill (0.0f);
        noiseGateLowState = noiseGateMidState = 0.0f;
        noiseGateGain = 1.0f;
        noiseGateHoldSamples = 0;
        noiseGateOpen = true;
    }

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

    // Once the global bypass fade has finished, avoid running the complete DSP
    // chain. The sanitized dry signal is already in the host buffer.
    if (! globalWet.isSmoothing() && globalWet.getCurrentValue() <= 0.00001f)
    {
        outputPeakDb.store (inputPeakDb.load (std::memory_order_relaxed), std::memory_order_relaxed);
        if (inputPeak >= 1.0f)
            outputClip.store (true, std::memory_order_relaxed);
        return;
    }

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
        constexpr float beats[] { 1.0f, 1.5f, 0.5f, 0.75f, 1.0f / 3.0f,
                                  0.25f, 0.375f, 1.0f / 6.0f };
        const auto division = juce::jlimit (0, 7, static_cast<int> (parameters.getRawParameterValue ("echoDivision")->load()));
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
    auto state = parameters.copyState();
    state.setProperty ("currentPresetIndex", currentPresetIndex, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destinationData);
}

void RockalizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (! xml->hasTagName (parameters.state.getType()))
            return;

        auto state = juce::ValueTree::fromXml (*xml);
        const auto presetToRestore = static_cast<int> (state.getProperty ("currentPresetIndex", 0));
        parameters.replaceState (state);

        // A named preset shown in the header must always sound exactly like
        // that preset. Interface-only input settings are preserved by loadPreset().
        if (! loadPreset (presetToRestore))
            loadPreset (0);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RockalizerAudioProcessor();
}
