#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
struct Preset { const char* name; float dose, tone, deess, mix, output; };
const Preset presets[] = {
    { "Default",      0.50f,  0.00f, 0.40f, 1.00f,  0.0f },
    { "Gentle Level", 0.25f,  0.10f, 0.30f, 1.00f,  0.0f },
    { "Rap Punch",    0.70f,  0.15f, 0.45f, 1.00f, -1.0f },
    { "Airy Pop",     0.55f,  0.60f, 0.55f, 1.00f, -1.0f },
    { "Dark Plug",    0.65f, -0.50f, 0.35f, 1.00f, -1.0f },
    { "Max Dose",     1.00f,  0.20f, 0.60f, 1.00f, -3.0f }
};
constexpr int numPresets = (int) (sizeof (presets) / sizeof (presets[0]));
}

VoxDoseProcessor::VoxDoseProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    doseP   = apvts.getRawParameterValue ("dose");
    toneP   = apvts.getRawParameterValue ("tone");
    deessP  = apvts.getRawParameterValue ("deess");
    mixP    = apvts.getRawParameterValue ("mix");
    outputP = apvts.getRawParameterValue ("output");
}

juce::AudioProcessorValueTreeState::ParameterLayout VoxDoseProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dose", 1 }, "Dose", juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "tone", 1 }, "Tone", juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "deess", 1 }, "De-ess", juce::NormalisableRange<float> (0.0f, 1.0f), 0.4f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 }, "Dry/Wet", juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "output", 1 }, "Output",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f));
    return layout;
}

void VoxDoseProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
}

bool VoxDoseProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
}

void VoxDoseProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    engine.process (buffer, doseP->load(), toneP->load(), deessP->load(), mixP->load(), outputP->load());
}

int VoxDoseProcessor::getNumPrograms() { return numPresets; }

const juce::String VoxDoseProcessor::getProgramName (int index)
{
    return (index >= 0 && index < numPresets) ? juce::String (presets[index].name) : juce::String();
}

void VoxDoseProcessor::setReal (const juce::String& id, float value)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
}

void VoxDoseProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= numPresets) return;
    currentProgram = index;
    const auto& p = presets[index];
    setReal ("dose", p.dose);   setReal ("tone", p.tone);   setReal ("deess", p.deess);
    setReal ("mix", p.mix);     setReal ("output", p.output);
}

void VoxDoseProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void VoxDoseProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* VoxDoseProcessor::createEditor()
{
    return new VoxDoseEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoxDoseProcessor();
}
