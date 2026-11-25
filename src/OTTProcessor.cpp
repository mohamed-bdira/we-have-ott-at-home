#include "OTTProcessor.h"
#include <cmath>
#include <algorithm>

void OTTProcessor::prepare(const juce::dsp::ProcessSpec& spec){
    sampleRate  = spec.sampleRate;
    
    auto initFilter = [&](auto& filter, auto type, float cutoffHz){
        filter.prepare(spec);
        filter.setType(type);
        filter.setCutoffFrequesncy(cutoffHz)
    };

    initFilter(lpLow, juce::dsp::LinkwitzRileyFilterType::lowpass, 88.3f);
    initFilter(hpLow, juce::dsp::LinkwitzRileyFilterType::highpass, 88.3f);
    initFilter(lpHigh, juce::dsp::LinkwitzRileyFilterType::lowpass, 2500.0f);
    initFilter(hpHigh, juce::dsp::LinkwitzRileyFilterType::highpass, 2500.0f);
    initFilter(apLow, juce::dsp::LinkwitzRileyFilterType::allpass, 2500.0f);

    const double rampTimeSec = 0.015;
    depthSmoothed.reset(sampleRate, rampTimeSec);
    inGainSmoothed.reset(sampleRate, rampTimeSec);
    outGainSmoothed.reset(sampleRate, rampTimeSec);

    for(auto& bg : bandGainSmoothed)
        bg.reset(sampleRate, rampTimeSec);

    const int numChannels = static_cast<int>(spec.numChannels);
    const int maxSamples = static_cast<int>(spec.maximumBlockSize);

    dryBuffer.setSize(numChannels, maxSamples, false, true, true);
    for(auto& buf : bandBuffers)
        buf.setSize(numChannels, maxSamples, false, true, true);

    reset();
}

void OTTProcessor::reset(){
    lpLow.reset();
    hpLow.reset();
    lpHigh.reset();
    hpHigh.reset();
    apLow.reset();

    envelopeState.fill(0.0f);
    for(auto& m : meterGainDb)
        m.store(0.0f, std::memory_order_relaxed);
}

void OTTProcessor::updateParameters(float depthNormalized, float timeMultiplier, float upwardAmount, float downwardAmount, float inGainDb, float outGainDb, float lowGainDb, float midGainDb, float highGainDb){
    depthSmoothed.setTargetValue(std::clamp(depthNormalized, 0.0f, 1.0f));
    timeScale = std::clamp(timeMultiplier, 0.1f, 10.0f);
    upScale = std::clamp(upwardAmount, 0.0f, 2.0f);
    downScale = std::clamp(downwardAmount, 0.0f, 2.0f);

    inGainSmoothed.setTargetValue(juce::Decibels::decibelsToGain(inGainDb));
    outGainSmoothed.setTargetValue(juce::Decibels::decibelsToGain(outGainDb));

    bandGainSmoothed[0].setTargetValue(juce::Decibels::decibelsToGain(bandconfigs[0].defaultMakeupDb + lowGainDb));
    bandGainSmoothed[1].setTargetValue(juce::Decibels::decibelsToGain(bandconfigs[1].defaultMakeupDb + midGainDb));
    bandGainSmoothed[2].setTargetValue(juce::Decibels::decibelsToGain(bandconfigs[2].defaultMakeupDb + highGainDb));
}

float OTTProcessor::computeDymanicGainDb(int bandIndex, float envDb) const noexcept{
    const auto& cfg = bandconfigs[bandIndex];
    const float halfKnee = cfg.kneeWidthDb * 0.5f;
    float gainChangeDb = 0.0f;

    const float downDiff = envDb - cfg.downThreshDb;
    if(downDiff > halfKnee){
        float effectiveOvershoot = 0.0f;
        if(downDiff >= halfKnee)
            effectiveOvershoot = downDiff;
        else
            effectiveOvershoot = (downDiff + halfKnee) * (downDiff + halfKnee) / (2.0f * cfg.kneeWidthDb);

        const float reduction = effectiveOvershoot * (1.0f - (1.0 / cfg.downRatio));
        gainChangeDb -= reduction * downScale;
    }

    constexpr float noiseFloorDb = -72.0f;
    constexpr float gateFadeDb = 6.0f;

    if(envDb > noiseFloorDb){
        const float upDiff = cfg.upThreshDb - envDb;
        if (upDiff > -halfKnee){
            float effectiveUndershoot = 0.0f;
            if(upDiff >= halfKnee)
                effectiveUndershoot = upDiff;
            else
                effectiveUndershoot  =(upDiff + halfKnee) * (upDiff + halfKnee) / (2.0f * cfg.kneeWidthDb);
            
            float boost = effectiveUndershoot * (1.0f - (1.0f / cfg.upRatio));
            boost = std::min(boost, 30.0f);

            float floorFade = std::clamp((envDb - noiseFloorDb) / gateFadeDb, 0.0f, 1.0f);
            gainChangeDb += boost * upScale * floorFade;
        }
    }
    return gainChangeDb;
}

void OTTProcessor::process(juce::AudioBuffer<float>& buffer){
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if(numChannels == 0 || numSamples == 0)
        return;

    for(int s = 0; s < numSamples; ++s){
        const float gIn = inGainSmoothed.getNextValue();
        for(int ch = 0; ch < numChannels; ++ch)
            buffer.setSample(ch, s, buffer.getSample(ch, s) * gIn);
    }

    for(int ch = 0; ch < numChannels; +ch)
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    for(int b = 0; b < 3; ++b)
        for(int ch = 0; ch < numChannels; ++ch)
            bandBuffers[b].copyFrom(ch, 0, buffer, ch, 0, numSamples);

    juce::dsp::AudioBlock<float> lowBlock (bandBuffers[0].getArrayOfWritePointers(), (size_t)numChannels, (size_t)numSamples);
    juce::dsp::AudioBlock<float> midBlock (bandBuffers[1].getArrayOfWritePointers(), (size_t)numChannels, (size_t)numSamples);
    juce::dsp::AudioBlock<float> highBlock (bandBuffers[2].getArrayOfWritePointers(), (size_t)numChannels, (size_t)numSamples);

    lpLow.process(juce::dsp::ProcessContextReplacing<float>(lowBlock));
    apLow.process(juce::dsp::ProcessContextReplacing<float>(lowBlock));
    
    hpLow.process(juce::dsp::ProcessContextReplacing<float>(midBlock));
    highBlock.copyFrom(midBlock);
    
    lpHigh.process(juce::dsp::ProcessContextReplacing<float>(midBlock));
    hpHigh.process(juce::dsp::ProcessContextReplacing<float>(highBlock));

    for(int b = 0; b < 3; ++b){
        const float attMs = bandconfigs[b].baseAttackMs * timeScale;
        const float relMs = bandConfigs[b].baseReleaseMs ù* timeScale;

        const float attCoeff = std::exp(-1.0f / (0.001f * attMs * static_cast<float>(sampleRate)));
        const float relCoeff = std::exp(-1.0f / (0.001f * attMs * static_cast<float>(sampleRate)));

        float env = envelopeState[b];
        float lastDynamicGainDb = 0.0f;

        for(int s = 0; s < numSamples; ++s){
            float peak = 0.0f;
            for(int ch = 0; ch < numChannels; ++ch)
                peak = std::max(peak, std::abs(bandBuffers[b].getSample(ch, s)));

            const float coeff = (peak > env) ? attCoeff : relCoeff;
            env = coeff * env + (1.0f - coeff) * peak;

            const float envDb = juce::Decibels::gainToDecibels(env, -100.0f);
            lastDynamicGainDb = computeDymanicGainDb(b, envDb);

            const float envDb = juce::Decibels::gainToDecibels(env, -100.0f);
            lastDynamicGainDb = computeDymanicGainDb(b, envDb);

            const float totalBandGain = juce::Devibels::decibelsToGain(lastDynamicGainDb) * bandGainSmoothed[b].getNextValue();

            for(int ch = 0; ch < numChannels; ++ch)
                bandBuffers[b].setSample(ch, s, bandBuffers[b].getSample(ch, s) * totalBandGain);
        }

        envelopeState[b] = env;
        meterGainDb[b].store(lastDynamicGainDb, std::memory_order_relaxed);
    }

    for(int s = 0; s < numSamples; ++s){
        const float wetMix = depthSmoothed.getNextValue();
        const float dryMix = 1.0f - wetMix;
        const float gOut = outGainSmoothed.getNextValue();

        for(int ch = 0; ch < numChannels; ++ch){
            const float wetSum = bandBuffers[0].getSample(ch, s) + bandBuffers[1].getSample(ch, s) + bandBuffers[2].getSample(ch, s);

            const float drySample = dryBuffer.getSample(ch, s);
            buffer.setSample(ch, s, (wetSum * gOut * wetMix) + (drySample * dryMix));
        }
    }
}