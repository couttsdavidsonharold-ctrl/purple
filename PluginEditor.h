#pragma once
#include "PluginProcessor.h"

// Hand-drawn, underground look: marker font, sketchy knobs, grungy background.
class UndergroundLnF : public juce::LookAndFeel_V4
{
public:
    UndergroundLnF();
    juce::Font marker (float height) const;

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;

private:
    juce::Typeface::Ptr typeface;
};

class PurpleEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit PurpleEditor (PurpleAudioProcessor&);
    ~PurpleEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void paintJuice (juce::Graphics&, juce::Rectangle<float> area);
    void paintSteps (juce::Graphics&, juce::Rectangle<float> area);
    void paintTitle (juce::Graphics&, juce::Rectangle<float> area);
    void rebuildBackground();

    PurpleAudioProcessor& proc;
    UndergroundLnF lnf;

    juce::Slider chopKnob, screwKnob, murkKnob, mixKnob, smoothKnob;
    juce::Label  chopLabel, screwLabel, murkLabel, mixLabel, smoothLabel;
    juce::ComboBox rateBox, patBox;

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAtt> chopAtt, screwAtt, murkAtt, mixAtt, smoothAtt;
    std::unique_ptr<ComboAtt>  rateAtt, patAtt;

    juce::Image background;
    juce::Rectangle<float> juiceArea, titleArea, stepsArea;

    // animation state (message thread only)
    float phaseA = 0.0f, phaseB = 0.0f, shownLevel = 0.0f, shownGate = 1.0f;
    int   shownStep = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurpleEditor)
};
