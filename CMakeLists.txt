cmake_minimum_required(VERSION 3.22)
project(PURPLE VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)   # Apple Silicon + Intel
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# ---- Formats -----------------------------------------------------------
set(PURPLE_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND PURPLE_FORMATS AU)
endif()

# AAX needs the official Avid SDK (free Avid developer account, NOT redistributable).
# Configure with:  -DAAX_SDK_PATH=/path/to/aax-sdk
if(AAX_SDK_PATH)
    juce_set_aax_sdk_path("${AAX_SDK_PATH}")
    list(APPEND PURPLE_FORMATS AAX)
    message(STATUS "AAX enabled, SDK: ${AAX_SDK_PATH}")
else()
    message(STATUS "AAX disabled (no -DAAX_SDK_PATH given)")
endif()

juce_add_plugin(PURPLE
    COMPANY_NAME "Hdax"
    PLUGIN_MANUFACTURER_CODE Hdax
    PLUGIN_CODE Prpl
    PRODUCT_NAME "PURPLE by Hdax"
    FORMATS ${PURPLE_FORMATS}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    AU_MAIN_TYPE kAudioUnitType_Effect
    AAX_CATEGORY AAX_ePlugInCategory_Effect
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx)

juce_add_binary_data(PurpleAssets SOURCES Assets/PermanentMarker-Regular.ttf)
set_target_properties(PurpleAssets PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_sources(PURPLE PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(PURPLE PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(PURPLE
    PRIVATE
        PurpleAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
