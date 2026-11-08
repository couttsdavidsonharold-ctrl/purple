#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
//  ScrewShifter : two-tap crossfaded delay-line pitch shifter (pitch down)
//==============================================================================
void ScrewShifter::prepare (double sampleRate, int numChannels)
{
    windowSize = juce::jmax (256, (int) std::lround (0.030 * sampleRate)); // 30 ms grain
    const int size = juce::nextPowerOfTwo (windowSize * 2 + 8);
    bufMask = size - 1;
    buf.assign ((size_t) numChannels, std::vector<float> ((size_t) size, 0.0f));
    writePos.assign ((size_t) numChannels, 0);
    phase = 0.0f;
    ratio = 1.0f;
}

void ScrewShifter::reset()
{
    for (auto& b : buf) std::fill (b.begin(), b.end(), 0.0f);
    std::fill (writePos.begin(), writePos.end(), 0);
    phase = 0.0f;
}

void ScrewShifter::processFrame (float* frame, int numCh) noexcept
{
    const float W = (float) windowSize;

    float p2 = phase + 0.5f;
    if (p2 >= 1.0f) p2 -= 1.0f;

    const float d1 = phase * W, d2 = p2 * W;
    const float w1 = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * phase);
    const float w2 = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * p2);

    for (int ch = 0; ch < numCh; ++ch)
    {
        auto& b = buf[(size_t) ch];
        int& wp = writePos[(size_t) ch];
        b[(size_t) wp] = frame[ch];

        auto tap = [&] (float d)
        {
            const float rp = (float) wp - d;
            const int   i0 = (int) std::floor (rp);
            const float fr = rp - (float) i0;
            const float a  = b[(size_t) (i0 & bufMask)];
            const float c  = b[(size_t) ((i0 + 1) & bufMask)];
            return a + fr * (c - a);
        };

        frame[ch] = w1 * tap (d1) + w2 * tap (d2);
        wp = (wp + 1) & bufMask;
    }

    // phase advance: delay grows by (1 - ratio) samples per sample
    float step = (1.0f - ratio) / W;
    const bool settling = step < 1.0e-6f;
    if (settling)
        step = (phase > 0.0f) ? 0.02f / W : 0.0f;   // glide back to the aligned point when Screw = 0

    phase += step;
    if (phase >= 1.0f) phase = settling ? 0.0f : phase - 1.0f;
}

//==============================================================================
//  Parameters
//==============================================================================
static const char* kPatterns[] = {
    "1010101010101010",   // Even
    "1101101101101101",   // Roll
    "1110111011101110",   // Skip
    "1011101110111011",   // Stutter
    "1111000011110000",   // Half-time
    "1001001010010010",   // Syncopated
    "1011010110110100",   // Swung
    "1100101001110100"    // Glitch
};
const char* getChopPattern (int i) { return kPatterns[juce::jlimit (0, 7, i)]; }
static const juce::StringArray kPatternNames { "Even", "Roll", "Skip", "Stutter", "Half-time", "Syncopated", "Swung", "Glitch" };
static const juce::StringArray kRateNames     { "1/4", "1/8", "1/16", "1/32", "1/8T", "1/16T" };
static const float kStepsPerQuarter[] = { 1.0f, 2.0f, 4.0f, 8.0f, 3.0f, 6.0f };

juce::AudioProcessorValueTreeState::ParameterLayout PurpleAudioProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    auto pct = [] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; };

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ids::chop, 1 },       "Chop",        NormalisableRange<float> (0.0f, 1.0f), 0.0f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    p.push_back (std::make_unique<AudioParameterChoice>(ParameterID { ids::chopRate, 1 },   "Chop Rate",   kRateNames, 2));
    p.push_back (std::make_unique<AudioParameterChoice>(ParameterID { ids::chopPat, 1 },    "Chop Pattern",kPatternNames, 0));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ids::chopSmooth, 1 }, "Chop Smooth", NormalisableRange<float> (0.0f, 1.0f), 0.25f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ids::screw, 1 },      "Screw",       NormalisableRange<float> (0.0f, 1.0f), 0.0f,
                 AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (-v * 12.0f, 1) + " st"; })));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ids::murk, 1 },       "Murk",        NormalisableRange<float> (0.0f, 1.0f), 0.0f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ids::mix, 1 },        "Mix",         NormalisableRange<float> (0.0f, 1.0f), 1.0f, AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
    return { p.begin(), p.end() };
}

//==============================================================================
PurpleAudioProcessor::PurpleAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PURPLE", createLayout())
{
}

bool PurpleAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto in = l.getMainInputChannelSet(), out = l.getMainOutputChannelSet();
    if (in != out) return false;
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void PurpleAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    const int nCh = juce::jmax (1, getTotalNumOutputChannels());

    shifter.prepare (sr, nCh);

    juce::dsp::ProcessSpec spec { sr, (juce::uint32) samplesPerBlock, (juce::uint32) nCh };
    dryDelay.prepare (spec);
    dryDelay.setDelay ((float) shifter.getLatencySamples());
    dryDelay.reset();

    murkFilter.prepare (spec);
    murkFilter.setType (juce::dsp::StateVariableTPTFilter<float>::Type::lowpass);
    murkFilter.reset();

    const double ramp = 0.03;
    smScrew.reset (sr, ramp);     smScrew.setCurrentAndTargetValue (*apvts.getRawParameterValue (ids::screw));
    smMurk.reset (sr, ramp);      smMurk.setCurrentAndTargetValue  (*apvts.getRawParameterValue (ids::murk));
    smMix.reset (sr, ramp);       smMix.setCurrentAndTargetValue   (*apvts.getRawParameterValue (ids::mix));
    smChop.reset (sr, ramp);      smChop.setCurrentAndTargetValue  (*apvts.getRawParameterValue (ids::chop));

    gateState = 1.0f;
    levelEnv = 0.0f;
    freeRunPpq = 0.0;

    setLatencySamples (shifter.getLatencySamples());
}

void PurpleAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh      = juce::jmin (buffer.getNumChannels(), 2);
    const int numSamples = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    // ---- host transport -------------------------------------------------
    double bpm = 120.0, hostPpq = 0.0;
    bool   hostSynced = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm(); b.hasValue() && *b > 1.0) bpm = *b;
            if (pos->getIsPlaying())
                if (auto q = pos->getPpqPosition(); q.hasValue()) { hostPpq = *q; hostSynced = true; }
        }

    // ---- parameters -----------------------------------------------------
    smScrew.setTargetValue (apvts.getRawParameterValue (ids::screw)->load());
    smMurk.setTargetValue  (apvts.getRawParameterValue (ids::murk)->load());
    smMix.setTargetValue   (apvts.getRawParameterValue (ids::mix)->load());
    smChop.setTargetValue  (apvts.getRawParameterValue (ids::chop)->load());

    const int   rateIdx  = juce::jlimit (0, 5, (int) apvts.getRawParameterValue (ids::chopRate)->load());
    const int   patIdx   = juce::jlimit (0, 7, (int) apvts.getRawParameterValue (ids::chopPat)->load());
    const float smoothP  = apvts.getRawParameterValue (ids::chopSmooth)->load();
    const char* pattern  = kPatterns[patIdx];
    const float stepsPerQ = kStepsPerQuarter[rateIdx];

    const double ppqPerSample = bpm / 60.0 / sr;
    const double latencyPpq   = ppqPerSample * shifter.getLatencySamples(); // align gate with delayed audio
    double ppq = hostSynced ? hostPpq : freeRunPpq;

    const float edgeMs = 0.5f + smoothP * 25.0f;
    const float gateCoef = 1.0f - std::exp (-1.0f / (0.001f * edgeMs * (float) sr));

    murkFilter.setCutoffFrequency (juce::jlimit (200.0f, 20000.0f, 20000.0f * std::pow (0.04f, smMurk.getNextValue())));
    smMurk.skip (numSamples - 1);

    float blockPeak = 0.0f;
    int lastStep = 0;

    for (int i = 0; i < numSamples; ++i)
    {
        float frame[2] = { 0.0f, 0.0f };
        for (int ch = 0; ch < numCh; ++ch) frame[ch] = buffer.getSample (ch, i);

        // dry path, delayed to match the shifter latency
        float dry[2] = { 0.0f, 0.0f };
        for (int ch = 0; ch < numCh; ++ch)
        {
            dryDelay.pushSample (ch, frame[ch]);
            dry[ch] = dryDelay.popSample (ch);
        }

        // SCREW
        const float screwV = smScrew.getNextValue();
        shifter.setRatio (std::exp2 (-screwV));        // screw 1.0 -> -12 semitones (half speed)
        shifter.processFrame (frame, numCh);

        // MURK (low-pass)
        for (int ch = 0; ch < numCh; ++ch)
            frame[ch] = murkFilter.processSample (ch, frame[ch]);

        // CHOP (tempo-synced gate)
        const double gp   = ppq - latencyPpq;
        const int    step = (int) std::floor (gp * (double) stepsPerQ);
        const int    idx  = ((step % 16) + 16) % 16;
        lastStep = idx;
        const float  depth = smChop.getNextValue();
        const float  target = 1.0f - depth * (pattern[idx] == '1' ? 0.0f : 1.0f);
        gateState += (target - gateState) * gateCoef;
        ppq += ppqPerSample;

        const float mix = smMix.getNextValue();
        for (int ch = 0; ch < numCh; ++ch)
        {
            const float wet = frame[ch] * gateState;
            const float out = dry[ch] * (1.0f - mix) + wet * mix;
            buffer.setSample (ch, i, out);
            blockPeak = juce::jmax (blockPeak, std::abs (out));
        }
    }

    if (! hostSynced) freeRunPpq = ppq;

    // meter for the UI animation (lock-free)
    levelEnv = juce::jmax (blockPeak, levelEnv * 0.9f);
    outputLevel.store (levelEnv, std::memory_order_relaxed);
    chopGain.store (gateState, std::memory_order_relaxed);
    chopStep.store (lastStep, std::memory_order_relaxed);
}

//==============================================================================
juce::AudioProcessorEditor* PurpleAudioProcessor::createEditor() { return new PurpleEditor (*this); }

void PurpleAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void PurpleAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PurpleAudioProcessor(); }
