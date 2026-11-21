#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "VoxDoseEngine.h"

class VoxDoseProcessor : public juce::AudioProcessor
{
public:
    VoxDoseProcessor();
    ~VoxDoseProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "VoxDose"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void setReal (const juce::String& id, float value);

    VoxDoseEngine engine;
    std::atomic<float>* doseP   = nullptr;
    std::atomic<float>* toneP   = nullptr;
    std::atomic<float>* deessP  = nullptr;
    std::atomic<float>* mixP    = nullptr;
    std::atomic<float>* outputP = nullptr;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxDoseProcessor)
};
