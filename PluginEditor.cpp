#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
    const juce::Colour kInk (0xff0a0610), kPanel (0xff1a0d2b), kJuice (0xff9b30ff), kJuiceLight (0xffd28bff),
                       kPaper (0xfff4eaff), kTape (0xffe8d9a0);

    // deterministic pseudo-random in [-1,1] so the sketchy lines never flicker
    float wob (int seed, int i)
    {
        const float v = std::sin ((float) seed * 12.9898f + (float) i * 78.233f) * 43758.5453f;
        return 2.0f * (v - std::floor (v)) - 1.0f;
    }

    juce::Path sketchCircle (juce::Point<float> c, float r, int seed, float jitter)
    {
        juce::Path p;
        const int n = 48;
        for (int i = 0; i <= n + 3; ++i)                      // overshoot a little, like a real pen stroke
        {
            const float a  = juce::MathConstants<float>::twoPi * (float) i / (float) n;
            const float rr = r * (1.0f + jitter * wob (seed, i % n));
            const juce::Point<float> pt (c.x + rr * std::sin (a), c.y - rr * std::cos (a));
            i == 0 ? p.startNewSubPath (pt) : p.lineTo (pt);
        }
        return p;
    }

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name, juce::Component& parent)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
        s.setColour (juce::Slider::textBoxTextColourId, kJuiceLight);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        l.setText (name, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, kPaper);
        parent.addAndMakeVisible (s);
        parent.addAndMakeVisible (l);
    }
}

//==============================================================================
UndergroundLnF::UndergroundLnF()
{
    typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::PermanentMarkerRegular_ttf,
                                                        (size_t) BinaryData::PermanentMarkerRegular_ttfSize);
    setColour (juce::ComboBox::textColourId, kPaper);
    setColour (juce::ComboBox::backgroundColourId, kPanel);
    setColour (juce::ComboBox::outlineColourId, kJuiceLight);
    setColour (juce::ComboBox::arrowColourId, kJuiceLight);
    setColour (juce::PopupMenu::backgroundColourId, kPanel);
    setColour (juce::PopupMenu::textColourId, kPaper);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, kJuice);
    setColour (juce::PopupMenu::highlightedTextColourId, kPaper);
}

juce::Font UndergroundLnF::marker (float h) const { return juce::Font (juce::FontOptions (typeface).withHeight (h)); }
juce::Font UndergroundLnF::getLabelFont (juce::Label& l)  { return marker (l.getFont().getHeight()); }
juce::Font UndergroundLnF::getComboBoxFont (juce::ComboBox&) { return marker (15.0f); }
juce::Font UndergroundLnF::getPopupMenuFont() { return marker (16.0f); }

void UndergroundLnF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (2.0f);
    g.setColour (kPanel);
    g.fillRoundedRectangle (r, 3.0f);
    for (int pass = 0; pass < 2; ++pass)                 // double, slightly offset pen outline
    {
        juce::Path p;
        p.startNewSubPath (r.getX() + wob (3 + pass, 0) * 2, r.getY() + wob (3 + pass, 1) * 2);
        p.lineTo (r.getRight() + wob (3 + pass, 2) * 2, r.getY() + wob (3 + pass, 3) * 2);
        p.lineTo (r.getRight() + wob (3 + pass, 4) * 2, r.getBottom() + wob (3 + pass, 5) * 2);
        p.lineTo (r.getX() + wob (3 + pass, 6) * 2, r.getBottom() + wob (3 + pass, 7) * 2);
        p.closeSubPath();
        g.setColour (kJuiceLight.withAlpha (pass ? 0.45f : 0.9f));
        g.strokePath (p, juce::PathStrokeType (1.5f));
    }
    juce::Path arrow;
    const float ax = r.getRight() - 14.0f, ay = r.getCentreY();
    arrow.addTriangle (ax - 5, ay - 3, ax + 5, ay - 3, ax, ay + 4);
    g.setColour (kJuiceLight);
    g.fillPath (arrow);
}

void UndergroundLnF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                       float startAngle, float endAngle, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.0f);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto  c = b.getCentre();
    const float a = startAngle + pos * (endAngle - startAngle);
    const int seed = (int) (reinterpret_cast<std::uintptr_t> (&s) % 97);

    // sketchy outer ring (3 pen passes)
    for (int pass = 0; pass < 3; ++pass)
    {
        g.setColour (kPaper.withAlpha (pass == 0 ? 0.85f : 0.35f));
        g.strokePath (sketchCircle (c, r, seed + pass * 7, 0.035f), juce::PathStrokeType (pass == 0 ? 2.2f : 1.2f));
    }

    // track + value "marker stroke"
    const float tr = r * 0.78f, th = r * 0.2f;
    juce::Path track, val;
    track.addCentredArc (c.x, c.y, tr, tr, 0.0f, startAngle, endAngle, true);
    val.addCentredArc   (c.x, c.y, tr, tr, 0.0f, startAngle, juce::jmax (startAngle + 0.02f, a), true);
    g.setColour (juce::Colour (0xff3d1a5c));
    g.strokePath (track, juce::PathStrokeType (th, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (kJuice);
    g.strokePath (val, juce::PathStrokeType (th, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (kJuiceLight.withAlpha (0.8f));
    g.strokePath (val, juce::PathStrokeType (th * 0.25f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // inner disc + chunky pointer
    g.setColour (kPanel);
    g.fillEllipse (c.x - r * 0.52f, c.y - r * 0.52f, r * 1.04f, r * 1.04f);
    g.setColour (kJuiceLight.withAlpha (0.7f));
    g.strokePath (sketchCircle (c, r * 0.52f, seed + 31, 0.05f), juce::PathStrokeType (1.5f));
    const juce::Point<float> tip (c.x + r * 0.46f * std::sin (a), c.y - r * 0.46f * std::cos (a));
    g.setColour (kPaper);
    g.drawLine (c.x + r * 0.12f * std::sin (a), c.y - r * 0.12f * std::cos (a), tip.x, tip.y, 4.5f);
}

//==============================================================================
PurpleEditor::PurpleEditor (PurpleAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);

    setupKnob (chopKnob,   chopLabel,   "CHOP",   *this);
    setupKnob (screwKnob,  screwLabel,  "SCREW",  *this);
    setupKnob (smoothKnob, smoothLabel, "smooth", *this);
    setupKnob (murkKnob,   murkLabel,   "murk",   *this);
    setupKnob (mixKnob,    mixLabel,    "mix",    *this);

    rateBox.addItemList (juce::StringArray { "1/4", "1/8", "1/16", "1/32", "1/8T", "1/16T" }, 1);
    patBox.addItemList  (juce::StringArray { "Even", "Roll", "Skip", "Stutter", "Half-time", "Syncopated", "Swung", "Glitch" }, 1);
    addAndMakeVisible (rateBox);
    addAndMakeVisible (patBox);

    auto& a = proc.apvts;
    chopAtt   = std::make_unique<SliderAtt> (a, ids::chop,       chopKnob);
    screwAtt  = std::make_unique<SliderAtt> (a, ids::screw,      screwKnob);
    smoothAtt = std::make_unique<SliderAtt> (a, ids::chopSmooth, smoothKnob);
    murkAtt   = std::make_unique<SliderAtt> (a, ids::murk,       murkKnob);
    mixAtt    = std::make_unique<SliderAtt> (a, ids::mix,        mixKnob);
    rateAtt   = std::make_unique<ComboAtt>  (a, ids::chopRate,   rateBox);
    patAtt    = std::make_unique<ComboAtt>  (a, ids::chopPat,    patBox);

    setResizable (true, true);
    setResizeLimits (600, 400, 1600, 1067);
    getConstrainer()->setFixedAspectRatio (1.5);
    setSize (780, 520);

    startTimerHz (30);   // UI-only animation; the audio thread only writes a few atomics
}

PurpleEditor::~PurpleEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void PurpleEditor::timerCallback()
{
    const float lvl  = proc.outputLevel.load (std::memory_order_relaxed);
    const float gate = proc.chopGain.load (std::memory_order_relaxed);
    shownStep = proc.chopStep.load (std::memory_order_relaxed);
    shownLevel += (lvl  - shownLevel) * 0.25f;
    shownGate  += (gate - shownGate)  * 0.35f;
    phaseA += 0.09f + shownLevel * 0.25f;
    phaseB += 0.057f;
    repaint (juiceArea.expanded (0, 30).toNearestInt());
    repaint (stepsArea.toNearestInt());
}

//==============================================================================
void PurpleEditor::rebuildBackground()
{
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;
    background = juce::Image (juce::Image::ARGB, w, h, true);
    juce::Graphics g (background);

    g.fillAll (kInk);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0x66651fb3), 0.0f, (float) h, juce::Colours::transparentBlack, (float) w * 0.6f, (float) h * 0.4f, true));
    g.fillAll();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0x44ff2bd6), (float) w, 0.0f, juce::Colours::transparentBlack, (float) w * 0.5f, (float) h * 0.5f, true));
    g.fillAll();

    juce::Random rnd (1234);
    for (int i = 0; i < w * h / 90; ++i)                       // grain / dust
    {
        g.setColour (juce::Colours::white.withAlpha (rnd.nextFloat() * 0.10f));
        g.fillRect (rnd.nextFloat() * (float) w, rnd.nextFloat() * (float) h, 1.2f, 1.2f);
    }
    for (int i = 0; i < 35; ++i)                               // scratches
    {
        const float x = rnd.nextFloat() * (float) w, y = rnd.nextFloat() * (float) h;
        g.setColour (juce::Colours::white.withAlpha (0.03f + rnd.nextFloat() * 0.07f));
        g.drawLine (x, y, x + rnd.nextFloat() * 90.0f - 20.0f, y + rnd.nextFloat() * 14.0f - 7.0f, 1.0f);
    }
    for (int row = 0; row < 14; ++row)                         // halftone dots, top-right corner
        for (int col = 0; col < 14; ++col)
        {
            const float rr = juce::jmax (0.0f, 3.2f - 0.23f * (float) (row + col)) * ((float) h / 520.0f);
            g.setColour (kJuice.withAlpha (0.55f));
            g.fillEllipse ((float) w - 14.0f - (float) col * 9.0f * (float) h / 520.0f, 10.0f + (float) row * 9.0f * (float) h / 520.0f, rr * 2, rr * 2);
        }
    g.setColour (kJuice.withAlpha (0.7f));                     // rough frame
    for (int pass = 0; pass < 2; ++pass)
    {
        juce::Path f;
        auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (5.0f);
        f.startNewSubPath (r.getX() + wob (9 + pass, 0) * 3, r.getY() + wob (9 + pass, 1) * 3);
        f.lineTo (r.getRight() + wob (9 + pass, 2) * 3, r.getY() + wob (9 + pass, 3) * 3);
        f.lineTo (r.getRight() + wob (9 + pass, 4) * 3, r.getBottom() + wob (9 + pass, 5) * 3);
        f.lineTo (r.getX() + wob (9 + pass, 6) * 3, r.getBottom() + wob (9 + pass, 7) * 3);
        f.closeSubPath();
        g.strokePath (f, juce::PathStrokeType (pass ? 1.0f : 2.2f));
    }
}

void PurpleEditor::paintTitle (juce::Graphics& g, juce::Rectangle<float> area)
{
    juce::Graphics::ScopedSaveState ss (g);
    g.addTransform (juce::AffineTransform::rotation (-0.05f, area.getCentreX(), area.getCentreY()));

    auto title = area.removeFromTop (area.getHeight() * 0.62f);
    g.setFont (lnf.marker (area.getWidth() * 0.30f));
    g.setColour (kJuice);                                                  // offset "spray" shadow
    g.drawFittedText ("PURPLE", title.translated (4.0f, 4.0f).toNearestInt(), juce::Justification::centred, 1, 0.6f);
    g.setColour (kPaper);
    g.drawFittedText ("PURPLE", title.toNearestInt(), juce::Justification::centred, 1, 0.6f);

    // spray drips
    g.setColour (kJuice);
    for (int i = 0; i < 4; ++i)
    {
        const float x = title.getX() + title.getWidth() * (0.18f + 0.2f * (float) i);
        const float len = 8.0f + 12.0f * (0.5f + 0.5f * wob (5, i));
        g.fillRoundedRectangle (x, title.getBottom() - 6.0f, 3.5f, len, 1.7f);
        g.fillEllipse (x - 1.0f, title.getBottom() - 6.0f + len - 3.0f, 5.5f, 6.0f);
    }

    g.addTransform (juce::AffineTransform::rotation (0.1f, area.getCentreX(), area.getCentreY()));
    g.setFont (lnf.marker (area.getHeight() * 0.58f));
    g.setColour (kJuiceLight);
    g.drawFittedText ("by Hdax", area.toNearestInt(), juce::Justification::centred, 1, 0.6f);

    juce::Path squig;                                                      // hand-drawn underline
    const float y = area.getBottom() - 2.0f;
    squig.startNewSubPath (area.getX() + area.getWidth() * 0.2f, y);
    for (float t = 0.0f; t <= 1.0f; t += 0.05f)
        squig.lineTo (area.getX() + area.getWidth() * (0.2f + 0.6f * t), y + std::sin (t * 18.0f) * 2.0f);
    g.setColour (kPaper.withAlpha (0.8f));
    g.strokePath (squig, juce::PathStrokeType (1.6f));
}

void PurpleEditor::paintSteps (juce::Graphics& g, juce::Rectangle<float> area)
{
    const int patIdx = (int) proc.apvts.getRawParameterValue (ids::chopPat)->load();
    const char* pat = getChopPattern (patIdx);
    const float cw = area.getWidth() / 16.0f;
    for (int i = 0; i < 16; ++i)
    {
        auto r = juce::Rectangle<float> (area.getX() + (float) i * cw + 2.0f, area.getY(), cw - 4.0f, area.getHeight())
                     .translated (wob (11, i) * 1.2f, wob (12, i) * 1.5f);
        const bool on = pat[i] == '1';
        g.setColour (on ? kJuice : juce::Colour (0xff2a1442));
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (kJuiceLight.withAlpha (on ? 0.9f : 0.45f));
        g.drawRoundedRectangle (r, 2.0f, 1.3f);
        if (i == shownStep)
        {
            g.setColour (kPaper);
            g.drawRoundedRectangle (r.expanded (2.0f), 3.0f, 2.4f);
        }
    }
}

void PurpleEditor::paintJuice (juce::Graphics& g, juce::Rectangle<float> area)
{
    juce::Graphics::ScopedSaveState ss (g);
    juce::Path glass;
    glass.addRoundedRectangle (area, 10.0f);
    g.reduceClipRegion (glass);

    g.setColour (juce::Colour (0xff07020d));
    g.fillPath (glass);

    const float fill = juce::jlimit (0.12f, 0.95f, 0.18f + shownLevel * 0.9f) * (0.55f + 0.45f * shownGate);
    const float top  = area.getBottom() - area.getHeight() * fill;

    for (int layer = 0; layer < 2; ++layer)
    {
        juce::Path wave;
        wave.startNewSubPath (area.getX(), area.getBottom());
        const float amp = (layer == 0 ? 7.0f : 4.5f) * (0.6f + shownLevel * 2.0f);
        for (float x = 0; x <= area.getWidth(); x += 4.0f)
        {
            const float ph = (layer == 0 ? phaseA : -phaseB) + x * 0.045f;
            wave.lineTo (area.getX() + x, top + std::sin (ph) * amp + (layer ? 6.0f : 0.0f));
        }
        wave.lineTo (area.getRight(), area.getBottom());
        wave.closeSubPath();
        g.setGradientFill (juce::ColourGradient (layer ? kJuice.darker (0.3f) : kJuiceLight.withAlpha (0.9f), 0, top,
                                                 kJuice.darker (0.8f), 0, area.getBottom(), false));
        g.fillPath (wave);
    }
    g.setColour (kJuiceLight.withAlpha (0.8f));
    for (int d = 0; d < 7; ++d)
    {
        const float x = area.getX() + area.getWidth() * (0.08f + 0.13f * (float) d);
        const float len = 6.0f + 12.0f * (0.5f + 0.5f * std::sin (phaseB * 1.3f + (float) d * 1.9f));
        g.fillRoundedRectangle (x, top - len, 4.0f, len + 3.0f, 2.0f);
    }
    g.setColour (kJuiceLight.withAlpha (0.6f));
    g.drawRoundedRectangle (area, 10.0f, 2.0f);
}

void PurpleEditor::paint (juce::Graphics& g)
{
    if (background.isValid()) g.drawImageAt (background, 0, 0);
    paintTitle (g, titleArea);
    paintSteps (g, stepsArea);
    paintJuice (g, juiceArea);
}

void PurpleEditor::resized()
{
    rebuildBackground();

    const float f = (float) getHeight() / 520.0f;
    auto b = getLocalBounds().reduced ((int) (20 * f));

    juiceArea = b.removeFromBottom ((int) (b.getHeight() * 0.20f)).toFloat();
    b.removeFromBottom ((int) (12 * f));

    auto left = b.removeFromLeft ((int) (b.getWidth() * 0.35f));
    titleArea = left.toFloat().withSizeKeepingCentre ((float) left.getWidth(), (float) left.getWidth() * 0.8f);
    b.removeFromLeft ((int) (14 * f));

    auto big = b.removeFromTop ((int) (b.getHeight() * 0.50f));
    const int bw = big.getWidth() / 2;
    auto chopCol = big.removeFromLeft (bw), screwCol = big;
    chopLabel.setFont  (lnf.marker (22.0f * f));  chopLabel.setBounds  (chopCol.removeFromTop ((int) (26 * f)));
    screwLabel.setFont (lnf.marker (22.0f * f));  screwLabel.setBounds (screwCol.removeFromTop ((int) (26 * f)));
    chopKnob.setBounds  (chopCol);
    screwKnob.setBounds (screwCol);

    stepsArea = b.removeFromTop ((int) (26 * f)).toFloat().reduced (0.0f, 3.0f * f);
    b.removeFromTop ((int) (6 * f));

    auto combos = b.removeFromTop ((int) (30 * f));
    rateBox.setBounds (combos.removeFromLeft (combos.getWidth() / 3).reduced (3));
    patBox.setBounds  (combos.removeFromLeft (combos.getWidth() / 2).reduced (3));

    const int kw = b.getWidth() / 3;
    auto place = [&] (juce::Label& l, juce::Slider& s, juce::Rectangle<int> r)
    { l.setFont (lnf.marker (15.0f * f)); l.setBounds (r.removeFromTop ((int) (18 * f))); s.setBounds (r); };
    place (smoothLabel, smoothKnob, b.removeFromLeft (kw));
    place (murkLabel,   murkKnob,   b.removeFromLeft (kw));
    place (mixLabel,    mixKnob,    b);
}
