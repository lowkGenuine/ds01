# DEESS-01 user guide

[Русский](USER_GUIDE.ru.md) · [Overview and download](../README.md) · [Releases](https://github.com/lowkGenuine/ds01/releases)

This guide covers **0.2.0**, with De-ess and Details pages. Control names match the English interface.

## Install and load

Use **DEESS-01-0.2.0-win-vst3.zip**, not a source-code archive. Extract it and copy the complete `DEESS-01.vst3` folder to `%CommonProgramFiles%\VST3`. Keep `Contents/Resources` and `Contents/x86_64-win` inside it. Enable VST3 in your host and rescan. See the [installation steps](../README.md#download-and-install).

## Find and reduce sibilance

1. Loop a phrase with the consonants you want to reduce.
2. Keep **Split** selected. Choose **Listen: Sibilance** and adjust Frequency until the offending region is easy to hear.
3. Return to **Listen: Processed**. Lower Threshold until Reduction moves on those consonants.
4. Set Range to the maximum attenuation you want. Listen to whole words, not just isolated “s” sounds.
5. Click **Active** to bypass. The button reads **Bypassed** while the original signal is passing; click it again to resume processing.

If the voice becomes dull or lisps, reduce Range, raise Threshold or narrow the band with Focus. Use **Listen: Removed** to hear what is being taken away, then return to Processed.

## Main controls

**Frequency** sets the centre of detection and processing. **Threshold** sets when reduction begins; lower values trigger more readily. **Range** limits attenuation even when the input is far above the threshold.

**Split** reduces the selected region, largely preserving the rest of the voice. **Wide** turns down the whole signal when the detector triggers. Both use Frequency and Focus to choose what to react to.

| Control | Range | Default |
|---|---|---|
| Frequency | 2.5–14 kHz | 6.00 kHz |
| Threshold | −54…0 dB | −25 dB |
| Range | 0–15 dB | 6 dB |
| Processing | Split / Wide | Split |

The meter shows detector-driven reduction. In 0.2.0 it does not account for Mix or bypass, so it can remain active while the output is dry. The spectrum displays the left input channel, or mono input; it is not a draggable control.

## Details and timing

**Details** opens the response controls. Switching pages preserves all sound settings; the spectrum and meter remain visible.

| Control | Range | Default | Effect |
|---|---|---|---|
| Focus | 0.5–6 Q | 2.20 Q | Higher Q narrows the selected band. |
| Attack | 0.05–10 ms | 0.35 ms | How quickly reduction starts. |
| Release | 15–250 ms | 65 ms | How quickly the level recovers. |
| Ratio | 1:1–12:1 | 4:1 | Compression above the threshold. |
| Mix | 0–100% | 100% | Blend the processed signal with the original. |
| Output | −12…+12 dB | 0 dB | Final output level. |

A faster attack catches brief peaks; a longer release holds attenuation longer. Listen to the complete phrase. Range still caps reduction at higher ratios.

## Monitoring and comparison

- **Listen: Processed:** the result, including Mix and Output.
- **Listen: Sibilance:** the selected detector band.
- **Listen: Removed:** the original minus the fully processed signal, before Mix and Output.

Sibilance and Removed ignore Mix for a clear diagnostic signal; Output still applies. Bypass takes priority and passes the original signal. At Mix = 0, Processed is dry apart from Output; leave Output at 0 dB for an unchanged dry level.

## Values, automation and saving

Drag a slider to change its value. Double-click the number to type; Frequency accepts `6.5 kHz` or `6500 Hz`. Use a decimal point. Double-click the slider track to restore that parameter's default.

All 12 parameters are automatable: the nine numeric controls, Processing, Listen and Bypass. The DAW project saves their values and the selected page. There is no separate preset browser or A/B memory.

The editor opens in a VST3 window. Hosts can also expose its parameters through their own controls.

## Troubleshooting

| Symptom | Check |
|---|---|
| Plug-in is missing | Use the Windows x64 plug-in archive, copy the entire bundle, enable VST3 and rescan. See the [installation guide](../README.md#download-and-install). |
| No audible reduction | Check Active, Processed, Mix, Range and Ratio. Lower Threshold and check Frequency with Sibilance monitoring. |
| Too much voice disappears | Use Split, narrow Focus, reduce Range or raise Threshold. |
| Output clipping appears | Lower Output or the input level. There is no output limiter. |
| Only high frequencies or a difference signal is audible | Return Listen to Processed. |

The plug-in reports zero samples of buffer latency; filter phase and detector response still apply. See [validation](VALIDATION.en.md) for measured checks and testing limits.
