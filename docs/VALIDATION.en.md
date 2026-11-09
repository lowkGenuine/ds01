# DEESS-01 0.2.0 validation

[Русский](VALIDATION.ru.md) · [User guide](USER_GUIDE.en.md)

Build: Windows x64, JUCE 7.0.12, LLVM-MinGW, Release VST3. The native editor is 840 × 300. These are engineering checks, not a listening comparison with commercial de-essers.

## Compiled VST3

The delivered binary was discovered, loaded and processed audio in a separate native VST3 test host. Checks cover all 12 exposed parameters, saved state, bypass, editor creation, mono/stereo and empty buffers.

At 44.1, 48, 96 and 192 kHz, with buffers of 1, 32, 257 and 4096 samples, it reports zero latency. At Range = 0, the test signal is preserved within 0.000002.

With Split and Range = 6 dB, a two-tone signal gave:

| Component | Measured level change |
|---|---:|
| 200 Hz | −0.012 dB |
| 6 kHz | −5.999 dB |

Wide attenuates the full signal when the detector triggers. Stereo balance is preserved.

## Editor and DSP

The screenshots render the actual C++ editor attached to the processor. Checks cover three main and six Details controls, frequency entry in Hz/kHz, parameter attachment, unchanged sound settings when switching pages, and saved page selection.

Core checks also cover silence, range limiting, Mix, detector/removed monitoring and stability during parameter changes. The meter in 0.2.0 shows detector-driven reduction before Mix and bypass; it is not a final-output attenuation meter.

## Remaining work

Full testing in the user's Ableton sessions and controlled listening comparisons remain open. The output has no limiter. The test log is retained in [CHECKS-0.2.0.txt](../CHECKS-0.2.0.txt).
