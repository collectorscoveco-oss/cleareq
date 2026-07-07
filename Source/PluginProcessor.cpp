#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr const char* bandIds[5] = { "low", "mud", "mid", "presence", "high" };
constexpr float defaultFreq[5] = { 80.0f, 260.0f, 1100.0f, 4200.0f, 12000.0f };
constexpr float defaultGain[5] = { 2.5f, -3.0f, 0.4f, 2.0f, 3.5f };
constexpr float defaultQ[5] = { 0.8f, 1.3f, 1.0f, 0.9f, 0.7f };
}

ClearEQAudioProcessor::ClearEQAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    bypassParam = apvts.getRawParameterValue ("bypass");
    outputParam = apvts.getRawParameterValue ("output");
    for (int i = 0; i < 5; ++i)
    {
        bandParams[(size_t) i * 3 + 0] = apvts.getRawParameterValue (juce::String (bandIds[i]) + "Freq");
        bandParams[(size_t) i * 3 + 1] = apvts.getRawParameterValue (juce::String (bandIds[i]) + "Gain");
        bandParams[(size_t) i * 3 + 2] = apvts.getRawParameterValue (juce::String (bandIds[i]) + "Q");
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout ClearEQAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterBool> ("bypass", "Bypass", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("output", "Output", juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f));

    const char* names[5] = { "Low", "Low-Mid Mud", "Mid", "Presence", "High" };
    for (int i = 0; i < 5; ++i)
    {
        auto prefix = juce::String (bandIds[i]);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Freq", juce::String (names[i]) + " Frequency", juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f), defaultFreq[i]));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Gain", juce::String (names[i]) + " Gain", juce::NormalisableRange<float> (-18.0f, 18.0f, 0.1f), defaultGain[i]));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (prefix + "Q", juce::String (names[i]) + " Q", juce::NormalisableRange<float> (0.2f, 8.0f, 0.01f, 0.35f), defaultQ[i]));
    }
    return { params.begin(), params.end() };
}

bool ClearEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    return mainOut == juce::AudioChannelSet::mono() || mainOut == juce::AudioChannelSet::stereo();
}

void ClearEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), static_cast<juce::uint32> (getTotalNumOutputChannels()) };
    leftChain.prepare (spec);
    rightChain.prepare (spec);
    updateFilters();
}

juce::dsp::IIR::Coefficients<float>::Ptr ClearEQAudioProcessor::makeCoefficients (int bandIndex, float freq, float gainDb, float q) const
{
    freq = juce::jlimit (20.0f, static_cast<float> (currentSampleRate * 0.45), freq);
    q = juce::jlimit (0.2f, 8.0f, q);
    const auto gain = juce::Decibels::decibelsToGain (gainDb);

    if (bandIndex == 0)
        return juce::dsp::IIR::Coefficients<float>::makeLowShelf (currentSampleRate, freq, q, gain);
    if (bandIndex == 4)
        return juce::dsp::IIR::Coefficients<float>::makeHighShelf (currentSampleRate, freq, q, gain);
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter (currentSampleRate, freq, q, gain);
}

void ClearEQAudioProcessor::updateBand (int bandIndex, float freq, float gainDb, float q)
{
    auto coeffs = makeCoefficients (bandIndex, freq, gainDb, q);
    switch (bandIndex)
    {
        case 0: *leftChain.get<0>().coefficients = *coeffs; *rightChain.get<0>().coefficients = *coeffs; break;
        case 1: *leftChain.get<1>().coefficients = *coeffs; *rightChain.get<1>().coefficients = *coeffs; break;
        case 2: *leftChain.get<2>().coefficients = *coeffs; *rightChain.get<2>().coefficients = *coeffs; break;
        case 3: *leftChain.get<3>().coefficients = *coeffs; *rightChain.get<3>().coefficients = *coeffs; break;
        case 4: *leftChain.get<4>().coefficients = *coeffs; *rightChain.get<4>().coefficients = *coeffs; break;
        default: break;
    }
}

void ClearEQAudioProcessor::updateFilters()
{
    for (int i = 0; i < 5; ++i)
        updateBand (i, bandParams[(size_t) i * 3]->load(), bandParams[(size_t) i * 3 + 1]->load(), bandParams[(size_t) i * 3 + 2]->load());
}

void ClearEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    if (bypassParam != nullptr && bypassParam->load() > 0.5f)
        return;

    updateFilters();
    juce::dsp::AudioBlock<float> block (buffer);
    auto leftBlock = block.getSingleChannelBlock (0);
    juce::dsp::ProcessContextReplacing<float> leftContext (leftBlock);
    leftChain.process (leftContext);

    if (buffer.getNumChannels() > 1)
    {
        auto rightBlock = block.getSingleChannelBlock (1);
        juce::dsp::ProcessContextReplacing<float> rightContext (rightBlock);
        rightChain.process (rightContext);
    }

    buffer.applyGain (juce::Decibels::decibelsToGain (outputParam != nullptr ? outputParam->load() : 0.0f));
}

juce::AudioProcessorEditor* ClearEQAudioProcessor::createEditor()
{
    return new ClearEQAudioProcessorEditor (*this);
}

void ClearEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void ClearEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClearEQAudioProcessor();
}
