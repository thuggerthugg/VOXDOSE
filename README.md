# VoxDose - one-knob vocal processor (JUCE)

Turn the big **DOSE** knob up for more of everything vocals usually need:
compression, mud cut, presence, air and soft saturation, all at once.

- **Tone**: darker <-> brighter
- **De-ess**: tames harsh "s" sounds (5-9 kHz)
- **Dry/Wet** and **Output**
- Presets in the dropdown (Default, Gentle Level, Rap Punch, Airy Pop, Dark Plug, Max Dose)

Chain inside: high-pass 80 Hz -> compressor -> EQ -> saturation -> de-esser -> mix/output.

## Get the .vst3 and .exe WITHOUT installing anything (GitHub builds it for you)
1. Make a free account at github.com and create a **New repository** (private is fine).
2. Click **uploading an existing file** and drag in everything from this folder, including the
   hidden `.github` folder. If `.github` does not upload, click **Add file > Create new file**,
   type `.github/workflows/build.yml` as the name and paste in that file's contents.
3. Open the **Actions** tab, wait a few minutes for **Build for Windows** to finish (green check).
4. Open the finished run and download **VoxDose-Windows** under *Artifacts*:
   - `VST3\VoxDose.vst3` -> copy into `C:\Program Files\Common Files\VST3\`, then in FL Studio
     use **Options > Manage plugins > Find installed plugins**
   - `Standalone\VoxDose.exe` -> a normal app to double-click and test

## Notes
- This code was written without being compiled or tested on my side. If the build shows a red X,
  copy the first error lines and send them back so they can be fixed.
- JUCE is free for personal use; check its license before selling or sharing the plugin.
