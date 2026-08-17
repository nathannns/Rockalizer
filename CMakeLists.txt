cmake_minimum_required(VERSION 3.22)

project(Rockalizer VERSION 0.65.0 LANGUAGES C CXX)

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
    Resources/Images/flanger_led_v1.png
    Resources/Images/logo_tape_v3.png
    Resources/Images/logo_chorus_v2.png
    Resources/Images/logo_echo_v2.png
    Resources/Images/logo_spring_v2.png
    Resources/Images/pedal_tape_v3.png
    Resources/Images/pedal_chorus_v2.png
    Resources/Images/pedal_echo_v2.png
    Resources/Images/pedal_spring_v2.png
    Resources/ImpulseResponses/Optimized/spring_space_clean.wav
    Resources/ImpulseResponses/Optimized/spring_9100_clean.wav
    Resources/ImpulseResponses/Optimized/spring_echomixer_clean.wav
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
        Source/DSP/TremoloModule.cpp
        Source/DSP/TremoloModule.h
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

option(ROCKALIZER_BUILD_TESTS "Build Rockalizer DSP validation tests" ON)
if(ROCKALIZER_BUILD_TESTS)
    juce_add_console_app(RockalizerDSPTests PRODUCT_NAME "Rockalizer DSP Tests")
    juce_generate_juce_header(RockalizerDSPTests)
    target_sources(RockalizerDSPTests PRIVATE
        Tests/DspValidation.cpp
        Source/DSP/ChorusModule.cpp Source/DSP/ChorusModule.h
        Source/DSP/EchoModule.cpp Source/DSP/EchoModule.h
        Source/DSP/SpringModule.cpp Source/DSP/SpringModule.h
        Source/DSP/TapeModule.cpp Source/DSP/TapeModule.h
        Source/DSP/TremoloModule.cpp Source/DSP/TremoloModule.h
    )
    target_include_directories(RockalizerDSPTests PRIVATE Source Source/DSP)
    target_compile_features(RockalizerDSPTests PRIVATE cxx_std_17)
    target_link_libraries(RockalizerDSPTests PRIVATE
        RockalizerData
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_events
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
    )
endif()
