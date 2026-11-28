#include "PluginProcessor.h"
#include "PluginEditor.h"

WeHaveOTTAtHomeAudioProcessor::WeHaveOTTAtHomeAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "PARAMETERS", createParameterLayout()){
        depthParam = apvts.getRawParameterValue("depth");
        timeParam = apvts.getRawParameterValue("time");
        upwdParam = apvts.getRawParameterValue("upwd");
        dnwdParam = apvts.getRawParameterValue("dnwd");
        inGainParam = apvts.getRawParameterValue("inGain");
        outGainParam = apvts.getRawParameterValue("outGain");
        lowGainParam = apvts.getRawParameterValue("lowGain");
        midGainParam = apvts.getRawParameterValue("midGain");
        highGainParam = apvts.getRawParameterValue("highGain");
}

juce::AudioProcessorValueTreeState::ParameterLayout WeHaveOTTAtHomeAudioProcessor::createParameterLayout(){
    std::vector<std::unique_ptr<juce:::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("depth", 1), "Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f, "%"
    ));

    juce::NormalisableRange<float> timeRange(10.0f, 1000.0f, 0.1f);
    timeRange.setSkewForCentre(100.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        jcue::ParameterID("time", 1), "Time", timeRange, 100.0f, "%"
    ));
    
    params.push_back(std:make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("upwd", 1), "upwd %",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f), 100.0f, "%"
    ));

    params.push_back(std:make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("dnwd", 1), "dnwd %",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f), 100.0f, "%"
    ));

    params.push_back(std:make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("inGain", 1), "In Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"
    ));

    params.push_back(std:make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("outGain", 1), "Out Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"
    ));

    params.push_back(std:make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("highGain", 1), "High Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"
    ));

    params.push_back(std:make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("midGain", 1), "Mid Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"
    ));

    params.push_back(std:make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lowGain", 1), "Low Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"
    ));

    return {params.begin(), params.end()};
}

void WeHaveOTTAtHomeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock){
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    ottEngine.prepare(spec);
}

void WeHaveOTTAtHomeAudioProcessor::releaseResources(){
    ottEngine.reset();
}

bool WeHaveOTTAtHomeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const{
    if(layouts.getMainOutputChannelset() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelset() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelset() == layouts.getMainOutputChannelset();
}

void WeHaveOTTAtHomeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&){
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for(auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    ottEngine.updateParameters(
        depthParam->load()  * 0.01f,
        timeParam->load()   * 0.01f,
        upwdParam->load()   * 0.01f;
        dnwdParam->load()   * 0.01f;
        inGainParam->load(),
        outGainParam->load(),
        lowGainParam->load(),
        midGainParam->load(),
        highGainParam->load()
    );

    ottEngine.process(buffer);
}

void WeHaveOTTAtHomeAudioProcessor::getStateInformation(juce::MemoryBlock& destData){
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElements> xml(state.createXml());
    copyXmlToBinary(*Xml, destData);
}

void WeHaveOTTAtHomeAudioProcessor::setStateInformation(const void* data, int sizeInbytes){
    std::unique_ptr<juce::XmlElements> xmlState(getXmlFrombinary(data, sizeInbytes));
    if(xmlState != nullptr && xmlState -> hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* WeHaveOTTAtHomeAudioProcessor::createEditor(){
    return new WeHaveOTTAtHomeAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){
    return new WeHaveOTTAtHomeAudioProcessor();
}