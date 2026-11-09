# Build DEESS-01

[Русский](BUILD.ru.md) · [Overview](../README.md)

Requirements: CMake 3.22 or newer, a C++17 toolchain and JUCE 7.0.12 source. The delivered platform is Windows x64 VST3. Builds for macOS and Linux have not been validated.

## Configure and build

Run from the repository root, replacing the JUCE path:

```powershell
cmake -S . -B build -DJUCE_PATH="C:/path/to/JUCE-7.0.12" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The VST3 bundle is generated under `build/Deess01_artefacts/Release/VST3/DEESS-01.vst3`. Install the whole bundle using the [user instructions](../README.md#download-and-install).

For MinGW/LLVM-MinGW, keep the compiler's `bin` directory on PATH during configuration and build: JUCE runs its manifest helper. The MinGW build statically links the C++ runtime.

## Checks

CTest runs the independent DSP test. The build also produces:

- `Deess01HostCheck`: pass the complete `.vst3` bundle path as its first argument.
- `Deess01Preview`: pass output PNG paths for the main and Details pages.

Executable locations depend on the generator: Visual Studio usually places them under `build/Release`; MinGW uses `build`.

To build just the DSP checks without JUCE:

```powershell
cmake -S . -B build-dsp -DDEESS01_BUILD_PLUGIN=OFF
cmake --build build-dsp --config Release
ctest --test-dir build-dsp -C Release --output-on-failure
```

See [validation](VALIDATION.en.md) for the checks performed on the delivered 0.2.0 binary, and the [JUCE notice](../ThirdParty/JUCE-LICENSE.md) for dependency terms. No separate project-source license is currently included.
