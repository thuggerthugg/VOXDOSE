#pragma once
#include "PluginProcessor.h"
#include <array>
#include <memory>

class VoxDoseEditor : public juce::AudioProcessorEditor
{
public:
    explicit VoxDoseEditor (VoxDoseProcessor&);
    ~VoxDoseEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    VoxDoseProcessor& proc;
    juce::LookAndFeel_V4 laf;

    juce::Label title, doseLabel;
    juce::ComboBox presetBox;
    juce::Slider doseKnob;
    std::unique_ptr<SliderAttachment> doseAtt;

    std::array<juce::Slider, 4> knobs;
    std::array<juce::Label, 4> knobLabels;
    std::array<std::unique_ptr<SliderAttachment>, 4> knobAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxDoseEditor)
};
