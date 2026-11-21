#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

// ------------------------------------------------------------------
//  One-knob vocal processor.
//  Chain: high-pass -> compressor -> mud cut / presence / air EQ
//         -> soft saturation -> de-esser -> dry/wet + output.
//  The DOSE amount drives compression depth, EQ lift and saturation together.
// ------------------------------------------------------------------
class VoxDoseEngine
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        sr = sampleRate;
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlock, 2 };
        hpf.prepare (spec); mud.prepare (spec); presence.prepare (spec); air.prepare (spec);
        comp.prepare (spec);
        *hpf.state = *Coeffs::makeHighPass (sr, 80.0f, 0.707f);
        for (auto& f : band)
        {
            f.prepare (spec);
            f.coefficients = Coeffs::makeBandPass (sr, 7000.0f, 1.2f);
        }
        dry.setSize (2, maxBlock);
        deessAtk = 1.0f - std::exp (-1.0f / (0.001f * (float) sr));
        deessRel = 1.0f - std::exp (-1.0f / (0.050f * (float) sr));
        reset();
    }

    void reset()
    {
        hpf.reset(); mud.reset(); presence.reset(); air.reset(); comp.reset();
        for (auto& f : band) f.reset();
        env = 0.0f;
        lastDose = -1.0f; lastTone = -2.0f;
    }

    void process (juce::AudioBuffer<float>& buffer, float dose, float tone,
                  float deess, float mix, float outDb)
    {
        const int n = buffer.getNumSamples();
        const int numCh = juce::jmin (2, buffer.getNumChannels());
        if (dry.getNumSamples() < n) dry.setSize (2, n, false, false, true);
        for (int ch = 0; ch < numCh; ++ch) dry.copyFrom (ch, 0, buffer, ch, 0, n);

        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);

        // 1. clean low end
        hpf.process (ctx);

        // 2. compression - deeper with more dose
        const float thr   = -12.0f - 18.0f * dose;
        const float ratio = 2.0f + 4.0f * dose;
        comp.setThreshold (thr);
        comp.setRatio (ratio);
        comp.setAttack (6.0f);
        comp.setRelease (120.0f);
        comp.process (ctx);
        buffer.applyGain (juce::Decibels::decibelsToGain (-thr * (1.0f - 1.0f / ratio) * 0.45f));

        // 3. EQ: cut mud, add presence and air (tone: -1 dark .. +1 bright)
        if (dose != lastDose || tone != lastTone)
        {
            lastDose = dose; lastTone = tone;
            *mud.state      = *Coeffs::makePeakFilter (sr, 300.0f, 1.0f,
                                  juce::Decibels::decibelsToGain (-2.5f * dose));
            *presence.state = *Coeffs::makePeakFilter (sr, 3200.0f, 0.9f,
                                  juce::Decibels::decibelsToGain (3.5f * dose));
            *air.state      = *Coeffs::makeHighShelf (sr, 10000.0f, 0.7f,
                                  juce::Decibels::decibelsToGain (6.0f * tone + 2.0f * dose));
        }
        mud.process (ctx); presence.process (ctx); air.process (ctx);

        // 4. soft saturation, level-matched
        const float drive = 1.0f + 3.0f * dose;
        const float satMix = 0.5f * dose;
        const float norm = 1.0f / std::tanh (drive);
        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* d = buffer.getWritePointer (ch);
            for (int i = 0; i < n; ++i)
            {
                const float y = std::tanh (d[i] * drive) * norm;
                d[i] = d[i] * (1.0f - satMix) + y * satMix;
            }
        }

        // 5. de-esser (reduces only the 5-9 kHz band when it gets loud)
        if (deess > 0.001f && numCh > 0)
        {
            const float thrGain = juce::Decibels::decibelsToGain (-20.0f - 20.0f * deess);
            float* l = buffer.getWritePointer (0);
            float* r = numCh > 1 ? buffer.getWritePointer (1) : buffer.getWritePointer (0);
            for (int i = 0; i < n; ++i)
            {
                const float bl = band[0].processSample (l[i]);
                const float br = band[1].processSample (r[i]);
                const float lvl = juce::jmax (std::abs (bl), std::abs (br));
                env += (lvl > env ? deessAtk : deessRel) * (lvl - env);
                const float g = env > thrGain ? std::pow (thrGain / env, 0.7f) : 1.0f;
                const float cutL = bl * (1.0f - g), cutR = br * (1.0f - g);
                l[i] -= cutL;
                if (numCh > 1) r[i] -= cutR;
            }
        }

        // 6. dry/wet and output
        const float outGain = juce::Decibels::decibelsToGain (outDb);
        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* w = buffer.getWritePointer (ch);
            auto* d = dry.getReadPointer (ch);
            for (int i = 0; i < n; ++i)
                w[i] = (d[i] * (1.0f - mix) + w[i] * mix) * outGain;
        }
    }

private:
    using Coeffs = juce::dsp::IIR::Coefficients<float>;
    using IIRDup = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, Coeffs>;

    double sr = 44100.0;
    IIRDup hpf, mud, presence, air;
    juce::dsp::Compressor<float> comp;
    juce::dsp::IIR::Filter<float> band[2];
    juce::AudioBuffer<float> dry;
    float env = 0.0f, deessAtk = 0.1f, deessRel = 0.001f;
    float lastDose = -1.0f, lastTone = -2.0f;
};
