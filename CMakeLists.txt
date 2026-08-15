cmake_minimum_required(VERSION 3.22)

project(Rockalizer VERSION 0.33.0 LANGUAGES C CXX)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.15
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(RockalizerData SOURCES
    Resources/Images/knob_v1.png
    Resources/Images/plugin_background.png
    Resources/Images/rockalizer_logo_v1.png
    Resources/Images/logo_tape_v3.png
    Resources/Images/logo_chorus_v2.png
    Resources/Images/logo_echo_v2.png
    Resources/Images/logo_spring_v2.png
    Resources/Images/pedal_tape_v3.png
    Resources/Images/pedal_chorus_v2.png
    Resources/Images/pedal_echo_v2.png
    Resources/Images/pedal_spring_v2.png
    Resources/ImpulseResponses/Clean/spring_gbsr_clean.wav
    Resources/ImpulseResponses/Clean/spring_deluxe_clean.wav
    Resources/ImpulseResponses/Clean/spring_space_clean.wav
    Resources/ImpulseResponses/Clean/spring_9100_clean.wav
    Resources/ImpulseResponses/Clean/spring_echomixer_clean.wav
    Resources/ImpulseResponses/Clean/spring_schaller_clean.wav
    Resources/ImpulseResponses/Clean/spring_pioneer_clean.wav
)

juce_add_plugin(Rockalizer
    COMPANY_NAME "Nathan Aji"
    BUNDLE_ID "com.nathanaji.rockalizer"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Rockalizer needs audio input access to process instruments and microphones."
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Nath
    PLUGIN_CODE Rkzr
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Rockalizer"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Images/rockalizer_app_icon.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Images/rockalizer_app_icon.png"
)

juce_generate_juce_header(Rockalizer)

target_sources(Rockalizer
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/DSP/ChorusModule.cpp
        Source/DSP/ChorusModule.h
        Source/DSP/EchoModule.cpp
        Source/DSP/EchoModule.h
        Source/DSP/TapeModule.cpp
        Source/DSP/TapeModule.h
        Source/DSP/SpringModule.cpp
        Source/DSP/SpringModule.h
)

target_compile_features(Rockalizer PRIVATE cxx_std_17)

target_compile_definitions(Rockalizer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Rockalizer
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        RockalizerData
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
