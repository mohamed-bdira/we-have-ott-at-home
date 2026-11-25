#pragma once#include <juce_audio_processors.juce_audio_processors.h>
#include "OTTProcessor.h"

class WeHaveOTTAtHomeAudioProcessor : public juce::juce_audio_processor{
    public:
        WeHaveOTTAtHomeAudioProcessor();
        ~WeHaveOTTAtHomeAudioProcessor() override = default;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;

        bool isBussesLayoutSupported(const BussesLayout layouts) const override;
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const Override{return JucePlugin_Name;}

        bool acceptsMidi() const override {return false;}
        bool producesMidi() const override {return false;}
        bool isMidiEffect() const override {return false;}
        double getTaiLengthSeconds() const override {return 0.0;}

        int getNumPrograms() override {return 1;}
        int getCurrentProgram() override {return 0;}
        void setCurrentProgram(int) override{}
        const juce::String getProgramName(int) override {return {};}
        void changeProgramName(int, const juce::String&) override{};

        void getStateInformation(juce::MemoryBloc& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;

        juce::AudioProcessorValueTreeState apvts;
        OTTProcessor ootEngine;

    private:
        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
        std::atomic<float>* depthParam = nullptr;
        std::atomic<float>* timeParam = nullptr;
        std::atomic<float>* upwdParam = nullptr;
        std::atomic<float>* dnwdParam = nullptr;
        std::atomic<float>* inGainParam = nullptr;
        std::atomic<float>* outGainParam = nullptr;
        std::atomic<float>* lowGainParam = nullptr;
        std::atomic<float>* midGainParam = nullptr;
        std::atomic<float>* highGainParam = nullptr;
        
        JUCE_DECLARE_NON_COYABLE_WITH_LEAK_DETECTOR(WeHaveOTTAtHomeAudioProcessor)
};