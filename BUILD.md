# Build

Requires CMake 3.22+, C++17 and JUCE 7.0.12 source. Supported artifact: Windows x64 VST3. macOS and Linux builds have not been validated.

```powershell
cmake -S . -B build -DJUCE_PATH=C:/path/to/JUCE-7.0.12 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

`Deess01HostCheck` takes the complete `.vst3` bundle path as its first argument. `Deess01Preview` takes an output PNG path. `Deess01DSPTest` exercises the independent processing core and can be built without JUCE by configuring with `-DDEESS01_BUILD_PLUGIN=OFF`.

For MinGW/LLVM-MinGW builds, ensure the compiler's `bin` directory is on PATH during configuration and build, since JUCE runs its manifest helper. This project statically links the plugin's C++ runtime on MinGW. A delivered plugin requires only the standard Windows system libraries.

Every public parameter has a stable versioned identifier. The processor supports mono-to-mono and stereo-to-stereo, host bypass, saved state and empty buffers. It allocates its delay buffers during `prepareToPlay`; normal audio processing uses no locks or dynamic allocation. The editor reads an atomic analysis buffer, separate from the processing path.
