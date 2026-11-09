cmake_minimum_required(VERSION 3.22)
project(Deess01 VERSION 0.2.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_testing()
add_executable(Deess01DSPTest Tests/dsp_test.cpp)
target_include_directories(Deess01DSPTest PRIVATE Source)
add_test(NAME dsp COMMAND Deess01DSPTest)
option(DEESS01_BUILD_PLUGIN "Build VST3, preview and native host checks" ON)
if(NOT DEESS01_BUILD_PLUGIN)
    return()
endif()
if(NOT DEFINED JUCE_PATH)
    message(FATAL_ERROR "Set JUCE_PATH to a JUCE 7.0.12 source directory")
endif()
add_subdirectory("${JUCE_PATH}" JUCE)
juce_add_plugin(Deess01 COMPANY_NAME "Shaper Audio"
    BUNDLE_ID "audio.shaper.deess01" PLUGIN_MANUFACTURER_CODE Shpr PLUGIN_CODE Ds01
    FORMATS VST3 PRODUCT_NAME "DEESS-01"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE IS_MIDI_EFFECT FALSE
    VST3_CATEGORIES Fx Dynamics COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(Deess01 PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(Deess01 PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_REPORT_APP_USAGE=0
    JUCE_MODAL_LOOPS_PERMITTED=1)
target_link_libraries(Deess01 PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags)
add_executable(Deess01Preview Tests/preview.cpp)
target_include_directories(Deess01Preview PRIVATE Source "${JUCE_PATH}/modules")
target_compile_definitions(Deess01Preview PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
target_link_libraries(Deess01Preview PRIVATE Deess01)
add_executable(Deess01HostCheck Tests/host_check.cpp)
target_compile_definitions(Deess01HostCheck PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
    JUCE_PLUGINHOST_VST3=1 JUCE_PLUGINHOST_VST=0 JUCE_MODAL_LOOPS_PERMITTED=1)
target_link_libraries(Deess01HostCheck PRIVATE juce::juce_audio_utils juce::juce_recommended_config_flags)
if(MINGW)
    foreach(t Deess01 Deess01DSPTest Deess01Preview Deess01HostCheck)
        target_link_options(${t} PUBLIC -static)
    endforeach()
endif()
