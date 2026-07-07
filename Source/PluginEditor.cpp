#include "PluginEditor.h"

namespace
{
float mapLog (float value, float minIn, float maxIn, float minOut, float maxOut)
{
    const auto norm = std::log (value / minIn) / std::log (maxIn / minIn);
    return minOut + norm * (maxOut - minOut);
}

float mapLogInv (float value, float minOut, float maxOut, float minIn, float maxIn)
{
    const auto norm = (value - minOut) / (maxOut - minOut);
    return minIn * std::pow (maxIn / minIn, norm);
}
}

ClearEQAudioProcessorEditor::ClearEQAudioProcessorEditor (ClearEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1180, 664);
    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio (16.0 / 9.0);
    getConstrainer()->setSizeLimits (860, 484, 1800, 1012);

    titleLabel.setText ("ClearEQ", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);

    subLabel.setText ("FAST MUSICAL EQUALIZER", juce::dontSendNotification);
    subLabel.setColour (juce::Label::textColourId, juce::Colour (0xff8793a6));
    subLabel.setFont (juce::FontOptions (12.0f));
    addAndMakeVisible (subLabel);

    presetBox.addItem ("Warm Vocal Cleanup", 1);
    presetBox.addItem ("Beat Bus Polish", 2);
    presetBox.addItem ("Quick Mud Cut", 3);
    presetBox.setSelectedId (1);
    addAndMakeVisible (presetBox);

    bypassButton.setClickingTogglesState (true);
    addAndMakeVisible (bypassButton);

    outputSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    outputSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);
    outputSlider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff5bbcff));
    addAndMakeVisible (outputSlider);

    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "bypass", bypassButton);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "output", outputSlider);

    startTimerHz (30);
}

void ClearEQAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (28);
    auto top = r.removeFromTop (72);
    titleLabel.setBounds (top.removeFromLeft (220).withHeight (34));
    subLabel.setBounds (42, 52, 260, 20);
    outputSlider.setBounds (getWidth() - 112, 20, 78, 70);
    bypassButton.setBounds (getWidth() - 222, 30, 86, 36);
    presetBox.setBounds (getWidth() - 560, 30, 300, 36);
    graphBounds = getLocalBounds().reduced (36).withTrimmedTop (98).withTrimmedBottom (138).toFloat();
}

void ClearEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bg (juce::Colour (0xff0f1520), 0, 0, juce::Colour (0xff07090e), (float) getWidth(), (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 28.0f);

    g.setColour (juce::Colour (0x22ffffff));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 28.0f, 1.5f);
    g.drawLine (28.0f, 86.0f, (float) getWidth() - 28.0f, 86.0f, 1.0f);

    auto logo = juce::Rectangle<float> (34, 25, 40, 40);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5bbcff), logo.getX(), logo.getY(), juce::Colour (0xffa78bfa), logo.getRight(), logo.getBottom(), false));
    g.fillRoundedRectangle (logo, 12.0f);
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.drawLine (logo.getX() + 10, logo.getY() + 16, logo.getRight() - 9, logo.getY() + 10, 2.0f);
    g.drawLine (logo.getX() + 10, logo.getY() + 28, logo.getRight() - 9, logo.getY() + 22, 2.0f);

    drawGraph (g);
    drawBandCards (g);
}

float ClearEQAudioProcessorEditor::getParam (int band, const juce::String& suffix) const
{
    if (auto* p = audioProcessor.apvts.getRawParameterValue (bandIds[(size_t) band] + suffix))
        return p->load();
    return 0.0f;
}

void ClearEQAudioProcessorEditor::setParam (int band, const juce::String& suffix, float value)
{
    if (auto* p = audioProcessor.apvts.getParameter (bandIds[(size_t) band] + suffix))
    {
        const auto norm = p->convertTo0to1 (value);
        p->beginChangeGesture();
        p->setValueNotifyingHost (norm);
        p->endChangeGesture();
    }
}

float ClearEQAudioProcessorEditor::frequencyToX (float freq) const { return mapLog (freq, 20.0f, 20000.0f, graphBounds.getX() + 26.0f, graphBounds.getRight() - 26.0f); }
float ClearEQAudioProcessorEditor::gainToY (float gainDb) const { return juce::jmap (gainDb, 18.0f, -18.0f, graphBounds.getY() + 38.0f, graphBounds.getBottom() - 32.0f); }
float ClearEQAudioProcessorEditor::pointToFrequency (float x) const { return mapLogInv (juce::jlimit (graphBounds.getX() + 26.0f, graphBounds.getRight() - 26.0f, x), graphBounds.getX() + 26.0f, graphBounds.getRight() - 26.0f, 20.0f, 20000.0f); }
float ClearEQAudioProcessorEditor::pointToGain (float y) const { return juce::jlimit (-18.0f, 18.0f, juce::jmap (y, graphBounds.getBottom() - 32.0f, graphBounds.getY() + 38.0f, -18.0f, 18.0f)); }
juce::Point<float> ClearEQAudioProcessorEditor::bandToPoint (int band) const { return { frequencyToX (getParam (band, "Freq")), gainToY (getParam (band, "Gain")) }; }

juce::Path ClearEQAudioProcessorEditor::buildResponsePath() const
{
    juce::Path p;
    for (int i = 0; i < 180; ++i)
    {
        const auto x = graphBounds.getX() + 26.0f + (graphBounds.getWidth() - 52.0f) * (float) i / 179.0f;
        const auto freq = pointToFrequency (x);
        float gain = 0.0f;
        for (int b = 0; b < bandCount; ++b)
        {
            const auto f = getParam (b, "Freq");
            const auto g = getParam (b, "Gain");
            const auto q = juce::jmax (0.25f, getParam (b, "Q"));
            const auto dist = std::abs (std::log2 (freq / f));
            gain += g * std::exp (-(dist * dist) * q * 1.25f);
        }
        const auto y = gainToY (juce::jlimit (-18.0f, 18.0f, gain));
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    return p;
}

void ClearEQAudioProcessorEditor::drawGraph (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff121824), graphBounds.getX(), graphBounds.getY(), juce::Colour (0xff090c12), graphBounds.getX(), graphBounds.getBottom(), false));
    g.fillRoundedRectangle (graphBounds, 24.0f);
    g.setColour (juce::Colour (0x24ffffff));
    g.drawRoundedRectangle (graphBounds, 24.0f, 1.2f);

    auto inner = graphBounds.reduced (26, 32).withTrimmedTop (16);
    g.setColour (juce::Colour (0x16ffffff));
    for (int i = 0; i <= 5; ++i)
    {
        const auto y = inner.getY() + inner.getHeight() * i / 5.0f;
        g.drawHorizontalLine ((int) y, inner.getX(), inner.getRight());
    }
    for (auto f : { 30.0f, 100.0f, 250.0f, 1000.0f, 4000.0f, 8000.0f, 16000.0f })
    {
        const auto x = frequencyToX (f);
        g.drawVerticalLine ((int) x, inner.getY(), inner.getBottom());
    }
    g.setColour (juce::Colour (0x35ffffff));
    g.drawHorizontalLine ((int) gainToY (0.0f), inner.getX(), inner.getRight());

    g.setColour (juce::Colour (0xffb7c3d8));
    g.setFont (14.0f);
    g.drawText ("Drag a node to shape the sound — labels explain what each area does.", graphBounds.reduced (22).removeFromTop (24), juce::Justification::left);

    auto response = buildResponsePath();
    g.setColour (juce::Colours::black.withAlpha (0.42f));
    g.strokePath (response, juce::PathStrokeType (8.0f));
    g.setColour (juce::Colour (0xff5bbcff));
    g.strokePath (response, juce::PathStrokeType (4.2f));

    g.setFont (12.0f);
    g.setColour (juce::Colour (0x77ffffff));
    for (auto label : { std::pair<const char*, float>{"30 Hz", 30.0f}, {"100", 100.0f}, {"250", 250.0f}, {"1k", 1000.0f}, {"4k", 4000.0f}, {"8k", 8000.0f}, {"16k", 16000.0f} })
        g.drawText (label.first, (int) frequencyToX (label.second) - 24, (int) graphBounds.getBottom() - 25, 54, 16, juce::Justification::centred);

    for (int b = 0; b < bandCount; ++b)
    {
        auto p = bandToPoint (b);
        auto colour = bandColours[(size_t) b];
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (p.x - 23, p.y - 21, 46, 46);
        g.setColour (colour);
        g.drawEllipse (p.x - 20, p.y - 20, 40, 40, 4.0f);
        g.fillEllipse (p.x - 10, p.y - 10, 20, 20);
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.drawEllipse (p.x - 10, p.y - 10, 20, 20, 1.4f);
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.setColour (juce::Colours::white);
        g.drawText (bandNames[(size_t) b], (int) p.x - 48, (int) p.y - (b == 1 ? -28 : 52), 96, 22, juce::Justification::centred);
    }
}

void ClearEQAudioProcessorEditor::drawBandCards (juce::Graphics& g)
{
    auto area = getLocalBounds().reduced (36).removeFromBottom (112).toFloat();
    const float gap = 12.0f;
    const float cardW = (area.getWidth() - gap * 5.0f) / 6.0f;
    g.setFont (13.0f);
    for (int b = 0; b < bandCount; ++b)
    {
        auto card = juce::Rectangle<float> (area.getX() + b * (cardW + gap), area.getY(), cardW, area.getHeight());
        g.setColour (juce::Colour (0x12ffffff));
        g.fillRoundedRectangle (card, 16.0f);
        g.setColour (juce::Colour (0x22ffffff));
        g.drawRoundedRectangle (card, 16.0f, 1.0f);
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.drawText (bandNames[(size_t) b] == "Mud" ? "Low-Mid Mud" : bandNames[(size_t) b], card.reduced (12).removeFromTop (22), juce::Justification::left);
        g.setColour (bandColours[(size_t) b]);
        g.fillEllipse (card.getRight() - 25, card.getY() + 17, 10, 10);
        g.setColour (juce::Colour (0xff8793a6));
        g.setFont (10.0f);
        g.drawText ("FREQ", card.getX()+12, card.getY()+54, 42, 14, juce::Justification::left);
        g.drawText ("GAIN", card.getX()+70, card.getY()+54, 42, 14, juce::Justification::left);
        g.drawText ("Q", card.getX()+126, card.getY()+54, 36, 14, juce::Justification::left);
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        auto freq = getParam (b, "Freq");
        g.drawText (freq >= 1000.0f ? juce::String (freq / 1000.0f, 1) + "k" : juce::String ((int) freq), card.getX()+12, card.getY()+70, 54, 20, juce::Justification::left);
        g.drawText (juce::String (getParam (b, "Gain"), 1), card.getX()+70, card.getY()+70, 54, 20, juce::Justification::left);
        g.drawText (juce::String (getParam (b, "Q"), 1), card.getX()+126, card.getY()+70, 54, 20, juce::Justification::left);
    }
    auto mix = juce::Rectangle<float> (area.getRight() - cardW, area.getY(), cardW, area.getHeight());
    g.setColour (juce::Colour (0x12ffffff));
    g.fillRoundedRectangle (mix, 16.0f);
    g.setColour (juce::Colour (0x22ffffff));
    g.drawRoundedRectangle (mix, 16.0f, 1.0f);
    g.setColour (juce::Colour (0xff8793a6));
    g.setFont (12.0f);
    g.drawText ("OUTPUT", mix.removeFromTop (34).toNearestInt(), juce::Justification::centred);
}

int ClearEQAudioProcessorEditor::hitTestBand (juce::Point<float> p) const
{
    for (int b = 0; b < bandCount; ++b)
        if (p.getDistanceFrom (bandToPoint (b)) < 28.0f)
            return b;
    return -1;
}

void ClearEQAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    draggedBand = hitTestBand (e.position);
}

void ClearEQAudioProcessorEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (draggedBand < 0) return;
    setParam (draggedBand, "Freq", pointToFrequency (e.position.x));
    setParam (draggedBand, "Gain", pointToGain (e.position.y));
    repaint();
}

void ClearEQAudioProcessorEditor::mouseUp (const juce::MouseEvent&) { draggedBand = -1; }
