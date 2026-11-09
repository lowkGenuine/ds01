# DEESS-01

[Русский](README.ru.md) · [Download VST3](https://github.com/lowkGenuine/ds01/releases) · [User guide](docs/USER_GUIDE.en.md)

DEESS-01 is a de-esser for controlling sharp consonants in a vocal recording. Choose the problem frequency, set when processing starts, and limit how much is removed. The monochrome interface keeps Frequency, Threshold and Range on the main screen; Details holds timing and mix controls.

**Download the plug-in:** [Windows x64 — v0.2.0](https://github.com/lowkGenuine/ds01/releases/download/v0.2.0/DEESS-01-0.2.0-win-vst3.zip).

**Code → Download ZIP**, **Source code**, and `DEESS-01-0.2.0-source.zip` contain source files for developers. They require compilation. For installation, choose the `win-vst3.zip` asset in [Releases](https://github.com/lowkGenuine/ds01/releases).

![DEESS-01 native interface](assets/Preview.png)

## Download and install

Download `DEESS-01-0.2.0-win-vst3.zip` and extract it. Copy the **entire `DEESS-01.vst3` folder** to `%CommonProgramFiles%\VST3`, normally `C:\Program Files\Common Files\VST3`. Keep its `Contents` folder intact. There is no separate installer; copying to the system folder may require administrator access.

In Ableton Live, enable the VST3 system folder in Settings → Plug-Ins and rescan. Add **DEESS-01** from the Plug-Ins browser to an audio track. See [Ableton's Windows VST guide](https://help.ableton.com/hc/en-us/articles/209071729-Using-VST-plug-ins-on-Windows) if it does not appear.

Close the DAW before replacing an installed build. Install only one copy of the same plug-in identity. This release provides Windows x64 VST3; macOS and Linux builds are not included.

[Download the English/Russian guides for offline reading](https://github.com/lowkGenuine/ds01/releases/download/v0.2.0/DEESS-01-0.2.0-guides-en-ru.zip).

## Features

- One dynamic frequency band; Frequency, Threshold and Range on the De-ess page.
- Split processing for the selected frequency region, or Wide for the whole voice.
- Processed, Sibilance and Removed monitoring.
- Focus, Attack, Release, Ratio, Mix and Output on the Details page.
- Input spectrum, gain-reduction meter and output-clipping indication.
- 12 automatable parameters, saved settings, mono and linked stereo.
- Zero samples of reported processing latency.

## Start with a voice

Loop a phrase with sharp “s” sounds. Choose **Listen: Sibilance** and adjust Frequency to find them. Return to **Listen: Processed**, lower Threshold until those consonants trigger reduction, then limit its depth with Range. Click **Active** to compare with the original signal.

The [guide](docs/USER_GUIDE.en.md) explains all controls, defaults and troubleshooting. The interface is in English; documentation is available in English and Russian.

## Latency and release status

Version 0.2.0 is a development build. It reports **0 samples of audio-buffer latency**. Filters still change phase, and the detector and attack/release settings have a response time. Output has no limiter.

Native checks cover VST3 loading, saved settings, mono/stereo, bypass, editor creation and signal processing. Listening comparisons and full Ableton session testing remain open. Presets, A/B slots, an external sidechain, lookahead controls and phoneme recognition are not implemented in this version.

[User guide](docs/USER_GUIDE.en.md) · [Validation](docs/VALIDATION.en.md) · [Build instructions](docs/BUILD.en.md)

## Source and dependencies

The project uses C++17 and JUCE 7.0.12. Dependency terms are recorded in [ThirdParty/JUCE-LICENSE.md](ThirdParty/JUCE-LICENSE.md). A separate license for the project's own source code has not been set in this repository.
