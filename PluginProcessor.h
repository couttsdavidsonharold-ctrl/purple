#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

namespace ids
{
    inline constexpr const char* chop      = "chop";       // 0..1  gate depth
    inline constexpr const char* chopRate  = "chopRate";   // choice: note division
    inline constexpr const char* chopPat   = "chopPat";    // choice: 16-step pattern
    inline constexpr const char* chopSmooth= "chopSmooth"; // 0..1  edge softness
    inline constexpr const char* screw     = "screw";      // 0..1  -> 0..-12 semitones
    inline constexpr const char* murk      = "murk";       // 0..1  low-pass darkening
    inline constexpr const char* mix       = "mix";        // 0..1
}

const char* getChopPattern (int index);   // 16 chars of '0'/'1'

// Real-time pitch shifter (two crossfaded delay taps). Pitch DOWN only (ratio <= 1).
class ScrewShifter
{
public:
    void prepare (double sampleRate, int numChannels);
    void reset();
    void setRatio (float r) noexcept { ratio = r; }
    // processes one stereo/mono frame in place; advances the phase ONCE per frame
    void processFrame (float* frame, int numCh) noexcept;
    int   getLatencySamples() const noexcept { return windowSize / 2; }

private:
    std::vector<std::vector<float>> buf;
    std::vector<int> writePos;
    int bufMask = 0, windowSize = 1024;
    float phase = 0.0f, ratio = 1.0f;
};

class PurpleAudioProcessor : public juce::AudioProcessor
{
public:
    PurpleAudioProcessor();
    ~PurpleAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock; // keep double overload hidden-safe

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "PURPLE by Hdax"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // read by the editor's animation (lock-free, written by audio thread)
    std::atomic<float> outputLevel { 0.0f };
    std::atomic<float> chopGain    { 1.0f };
    std::atomic<int>   chopStep    { 0 };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    ScrewShifter shifter;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> dryDelay { 16384 };
    juce::dsp::StateVariableTPTFilter<float> murkFilter;
    juce::SmoothedValue<float> smScrew, smMurk, smMix, smChop, smChopGain;

    double sr = 44100.0;
    double freeRunPpq = 0.0;
    float  gateState = 1.0f;
    float  levelEnv = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurpleAudioProcessor)
};
