cmake_minimum_required(VERSION 3.22)

project(TR808Synth VERSION 0.1.0 LANGUAGES C CXX)

# ---------------------------------------------------------------------------
# Standards / build settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a Release build for single-config generators (Ninja, Makefiles).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# JUCE (fetched automatically — no git submodule needed)
# Downloads land in the build tree (_deps), which lives outside OneDrive when
# you configure with e.g. -B C:/dev/808-app/build.
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.13
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# Plugin target
#   AU is only produced on macOS; JUCE silently skips it elsewhere, so listing
#   it here keeps the same CMakeLists building on Windows (VST3 + Standalone)
#   and on a Mac later (AU + VST3 + Standalone).
# ---------------------------------------------------------------------------
juce_add_plugin(TR808Synth
    COMPANY_NAME                "Sagiv"
    BUNDLE_ID                   "com.sagiv.tr808synth"
    PLUGIN_MANUFACTURER_CODE    Sgv8
    PLUGIN_CODE                 T808
    FORMATS                     AU VST3 Standalone
    PRODUCT_NAME                "TR-808 Synth"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE   # avoids needing admin to write to the system VST3 dir
    VST3_CATEGORIES             "Instrument" "Drum"
    AU_MAIN_TYPE                "kAudioUnitType_MusicDevice"
)

# Reusable DSP building blocks (M1). Compiled into both the plugin and the
# offline test app — they depend only on juce_core/juce_audio_basics/juce_dsp,
# never on any UI, so each block is independently testable.
set(TR808_DSP_SOURCES
    Source/dsp/BandlimitedOsc.cpp
    Source/dsp/NoiseGen.cpp
    Source/dsp/Envelope.cpp
    Source/dsp/PitchEnvelope.cpp
    Source/dsp/SVFilter.cpp
    Source/dsp/ResonatorBT.cpp
    Source/dsp/Saturator.cpp
    Source/dsp/MetalCluster.cpp
)

# Voice engine (M2): the 16 voices + manager. Pure-DSP voices; the manager uses
# juce_audio_basics (MidiBuffer/AudioBuffer). Compiled into the plugin and the
# voice test app.
set(TR808_VOICE_SOURCES
    Source/voices/BassDrumVoice.cpp
    Source/voices/SnareVoice.cpp
    Source/voices/TomCongaVoice.cpp
    Source/voices/TunedResonatorVoice.cpp
    Source/voices/ClapVoice.cpp
    Source/voices/CowbellVoice.cpp
    Source/voices/MetalVoice.cpp
    Source/voices/MaracasVoice.cpp
    Source/engine/VoiceManager.cpp
    Source/engine/Mixer.cpp
)

target_sources(TR808Synth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/params/ParameterLayout.cpp
    Source/engine/Sequencer.cpp
    ${TR808_DSP_SOURCES}
    ${TR808_VOICE_SOURCES}
)

target_include_directories(TR808Synth PRIVATE Source)

target_compile_definitions(TR808Synth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    # NOTE: JUCE_DISPLAY_SPLASH_SCREEN is intentionally NOT set to 0 — disabling
    # the splash requires a paid JUCE licence. Flip it later if you hold one.
)

target_link_libraries(TR808Synth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ---------------------------------------------------------------------------
# Offline DSP test harness (M1). Build with -DTR808_BUILD_TESTS=ON (default).
# Run the resulting console app; it exits non-zero if any check fails.
# ---------------------------------------------------------------------------
option(TR808_BUILD_TESTS "Build the offline DSP test harness" ON)

if(TR808_BUILD_TESTS)
    juce_add_console_app(dsp_tests PRODUCT_NAME "TR808 DSP Tests")

    target_sources(dsp_tests PRIVATE
        tests/DspTests.cpp
        ${TR808_DSP_SOURCES}
    )

    target_include_directories(dsp_tests PRIVATE Source)

    target_compile_definitions(dsp_tests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
    )

    target_link_libraries(dsp_tests
        PRIVATE
            juce::juce_dsp
            juce::juce_audio_basics
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )

    # Voice-engine tests (M2): triggers each voice, checks output, GM mapping,
    # and the hi-hat choke.
    juce_add_console_app(voice_tests PRODUCT_NAME "TR808 Voice Tests")

    target_sources(voice_tests PRIVATE
        tests/VoiceTests.cpp
        ${TR808_DSP_SOURCES}
        ${TR808_VOICE_SOURCES}
    )

    target_include_directories(voice_tests PRIVATE Source)

    target_compile_definitions(voice_tests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
    )

    target_link_libraries(voice_tests
        PRIVATE
            juce::juce_dsp
            juce::juce_audio_basics
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )

    # Sequencer tests (M4): step timing, swing, accent/flam, mute/solo,
    # host vs internal transport, and state round-trip.
    juce_add_console_app(seq_tests PRODUCT_NAME "TR808 Sequencer Tests")

    target_sources(seq_tests PRIVATE
        tests/SeqTests.cpp
        Source/engine/Sequencer.cpp
    )

    target_include_directories(seq_tests PRIVATE Source)

    target_compile_definitions(seq_tests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
    )

    target_link_libraries(seq_tests
        PRIVATE
            juce::juce_dsp
            juce::juce_audio_basics
            juce::juce_data_structures
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )
endif()
