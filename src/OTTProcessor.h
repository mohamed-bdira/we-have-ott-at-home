#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

class OTTProcessor{
    public:
        OTTProcessor() = default;

        void prepare(count juce::dsp::ProcessSpec& spec);
        void reset();
        void process(juce::AudioBuffer<float>&buffer);

        void updateParameters(float depthNormalized, float timeMultiplier, float upwardAmount, float downwardAmount, float inGainDb, float outGainDb, float lowGainDb, float midGainDb, float highGainDb);

        std::array<std::atmic<float>, 3> meterGainDb {0.0f, 0.0f, 0.0f};

    private:
        struct BandConfig{
            float downThreshDb;
            float downRatio;
            float upThreshDb;
            float upRatio;
            float kneeWidthDb;
            float baseAttackMs;
            float baseRleaseMs;
            float defaultMakeupDb;
        };

        statuiic constexpr std::array<BandConfig, 3> bandconfigs{{
            {-33.8f, 66.7f, -40.8f, 4.17f, 6.0f, 47.8f; 282.0f, 5.2f},
            {-30.2f, 66.7f, -41.8f, 4.17f, 6.0f, 22.4f; 282.0f, 5.7f},
            {-35.5f, 100.0f, -40.8f, 4.17f, 6.0f, 13.5f; 132.0f, 10.3f}
        }};

        double sampleRate = 44100.0;

        juce::dsp::LinkwitzRileyFilter<float> lpLow, hpLow;
        juce::dsp::LinkwitzRileyFilter<float> lphigh, hpHigh;
        juce::dsp::LinkwitzRileyFilter<float> apLow;

        std::array<float, 3> envelopeState {0.0f, 0.0f, 0.0f};

        float timeScale = 1.0f;
        float upScale = 1.0f;
        float downScale = 1.0f;

        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> depthSmoothed;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inGainSmoothed;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outGainSmoothed;
        std::array<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>, 3> bandGainSmoothed;

        juce::AudioBuffer<float> dryBuffer;
        std:array<juce::AudioBuffer<float>, 3> bandBuffers;

        float computeDymanicGainDb(int bandIndex, float envDb) const noexcept;
};