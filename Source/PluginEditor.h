#pragma once

#include "JuceHeader.h"
#include "PluginProcessor.h"

class ClearEQAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit ClearEQAudioProcessorEditor (ClearEQAudioProcessor&);
    ~ClearEQAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    ClearEQAudioProcessor& audioProcessor;
    juce::TextButton bypassButton { "Bypass" };
    juce::TextButton deltaButton { "Delta" };
    juce::ComboBox presetBox;
    juce::Slider outputSlider;
    juce::Label titleLabel, subLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deltaAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;

    int draggedBand = -1;
    juce::Rectangle<float> graphBounds;

    static constexpr int bandCount = 5;
    const std::array<juce::String, bandCount> bandNames { "Low", "Mud", "Mid", "Presence", "High" };
    const std::array<juce::Colour, bandCount> bandColours {
        juce::Colour (0xff45f0d0), juce::Colour (0xff5bbcff), juce::Colour (0xffffcf65), juce::Colour (0xffa78bfa), juce::Colour (0xffff6fae)
    };
    const std::array<juce::String, bandCount> bandIds { "low", "mud", "mid", "presence", "high" };

    void timerCallback() override { repaint(); }
    float getParam (int band, const juce::String& suffix) const;
    void setParam (int band, const juce::String& suffix, float value);
    void setDeltaBand (int band);
    bool isDeltaEnabled() const;
    int getDeltaBand() const;
    juce::Point<float> bandToPoint (int band) const;
    float pointToFrequency (float x) const;
    float pointToGain (float y) const;
    float frequencyToX (float freq) const;
    float gainToY (float gainDb) const;
    juce::Path buildResponsePath() const;
    void drawGraph (juce::Graphics&);
    void drawBandCards (juce::Graphics&);
    int hitTestBand (juce::Point<float> p) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClearEQAudioProcessorEditor)
};
