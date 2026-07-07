#pragma once

#include "JuceHeader.h"

class ClearEQAudioProcessor final : public juce::AudioProcessor
{
public:
    ClearEQAudioProcessor();
    ~ClearEQAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using MonoChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter, Filter>;

    MonoChain leftChain, rightChain;
    double currentSampleRate = 44100.0;
    std::atomic<float>* bypassParam = nullptr;
    std::atomic<float>* outputParam = nullptr;
    std::array<std::atomic<float>*, 15> bandParams{};

    void updateFilters();
    void updateBand (int bandIndex, float freq, float gainDb, float q);
    juce::dsp::IIR::Coefficients<float>::Ptr makeCoefficients (int bandIndex, float freq, float gainDb, float q) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClearEQAudioProcessor)
};
