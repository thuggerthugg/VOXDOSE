#include "PluginEditor.h"

VoxDoseEditor::VoxDoseEditor (VoxDoseProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    laf.setColourScheme (juce::LookAndFeel_V4::getMidnightColourScheme());
    laf.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffb86bff));
    laf.setColour (juce::Slider::thumbColourId, juce::Colour (0xfff1e4ff));
    setLookAndFeel (&laf);
    setSize (620, 440);

    title.setText ("VOXDOSE", juce::dontSendNotification);
    title.setFont (juce::Font (28.0f, juce::Font::bold));
    addAndMakeVisible (title);

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    doseKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    doseKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 110, 26);
    doseKnob.setTextBoxIsEditable (false);
    addAndMakeVisible (doseKnob);
    doseAtt = std::make_unique<SliderAttachment> (proc.apvts, "dose", doseKnob);
    doseKnob.textFromValueFunction = [] (double v) { return juce::String ((int) std::round (v * 100.0)) + " %"; };
    doseKnob.updateText();

    doseLabel.setText ("DOSE", juce::dontSendNotification);
    doseLabel.setFont (juce::Font (20.0f, juce::Font::bold));
    doseLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (doseLabel);

    const char* ids[4]    = { "tone", "deess", "mix", "output" };
    const char* labels[4] = { "Tone", "De-ess", "Dry/Wet", "Output" };
    for (size_t i = 0; i < 4; ++i)
    {
        knobs[i].setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knobs[i].setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
        knobs[i].setTextBoxIsEditable (false);
        addAndMakeVisible (knobs[i]);
        knobLabels[i].setText (labels[i], juce::dontSendNotification);
        knobLabels[i].setJustificationType (juce::Justification::centred);
        addAndMakeVisible (knobLabels[i]);
        knobAtt[i] = std::make_unique<SliderAttachment> (proc.apvts, ids[i], knobs[i]);
    }
    // set text functions after the attachments (they install their own)
    knobs[0].textFromValueFunction = [] (double v)
    {
        return v < -0.05 ? juce::String ("Darker") : v > 0.05 ? juce::String ("Brighter") : juce::String ("Neutral");
    };
    knobs[1].textFromValueFunction = [] (double v) { return juce::String ((int) std::round (v * 100.0)) + " %"; };
    knobs[2].textFromValueFunction = [] (double v) { return juce::String ((int) std::round (v * 100.0)) + " %"; };
    knobs[3].textFromValueFunction = [] (double v) { return juce::String (v, 1) + " dB"; };
    for (auto& k : knobs) k.updateText();
}

VoxDoseEditor::~VoxDoseEditor()
{
    setLookAndFeel (nullptr);
}

void VoxDoseEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1a0f33), 0.0f, 0.0f,
                                             juce::Colour (0xff08050f), 0.0f, (float) getHeight(), false));
    g.fillAll();

    g.setColour (juce::Colour (0xff22163f));
    g.fillRoundedRectangle (juce::Rectangle<float> (16.0f, 250.0f, (float) getWidth() - 32.0f, 170.0f), 12.0f);
    g.setColour (juce::Colour (0xff3b2a66));
    g.drawRoundedRectangle (juce::Rectangle<float> (16.0f, 250.0f, (float) getWidth() - 32.0f, 170.0f), 12.0f, 1.0f);
}

void VoxDoseEditor::resized()
{
    auto area = getLocalBounds().reduced (16);

    auto top = area.removeFromTop (46);
    title.setBounds (top.removeFromLeft (260));
    presetBox.setBounds (top.removeFromRight (200).reduced (0, 8));

    auto doseArea = area.removeFromTop (190);
    doseLabel.setBounds (doseArea.removeFromTop (28));
    doseKnob.setBounds (doseArea.withSizeKeepingCentre (220, doseArea.getHeight()));

    area.removeFromTop (20);
    auto row = area.reduced (8, 10);
    const int w = row.getWidth() / 4;
    for (size_t i = 0; i < 4; ++i)
    {
        auto k = row.removeFromLeft (w);
        knobLabels[i].setBounds (k.removeFromTop (22));
        knobs[i].setBounds (k.reduced (8, 0));
    }
}
