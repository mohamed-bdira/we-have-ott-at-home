cmake_minimum_required(VERSION 3.22)

project(WeHaveOTTAtHome VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

#Add JUCE
add_subdirectory(libs/JUCE)

juce_add_plugin(WeHaveOTTAtHome
    COMPANY_NAME "Haramonics"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Atha
    PLUGIN_CODE Wott
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "We Have OTT At Home"
)

target_source(WeHaveOTTAtHome
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/OTTProcessor.cpp
)

target_compile_definitions(WeHaveOTTAtHome
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST=0
)

target_link_librarties(WeHaveOTTAtHome
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lfo_flags
        juce::juce_recommended_warning_flags
)