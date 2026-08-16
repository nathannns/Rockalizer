# Rockalizer v0.41 profiling and validation

## Release build and automated DSP matrix

```bash
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DROCKALIZER_BUILD_TESTS=ON
cmake --build build-release
./build-release/RockalizerDSPTests_artefacts/Release/Rockalizer\ DSP\ Tests
```

The validation executable runs mono and stereo at 44.1, 48, 88.2 and 96 kHz,
with block sizes 32, 64, 128, 256, 512 and 1024. It stresses Tape, Doubler,
Chorus/Flanger and high-feedback/high-Mix Echo, rejecting NaN, infinity and
runaway output.

## Xcode Instruments: Cubase realistic-session profile

1. Build Release and open Cubase with a normal guitar project.
2. Use at least eight Rockalizer instances. Keep four fully active and bypass
   different modules on the other four.
3. In Instruments choose **Time Profiler**, target Cubase, set the recording
   duration to 60 seconds and enable **Separate by Thread** and **Hide System
   Libraries**.
4. Record these passes at 48 kHz/64 samples: all effects active; Spring bypassed;
   all creative modules bypassed; rapid preset changes; Echo at 90% Mix and 82%
   Repeats.
5. Repeat the worst pass at 44.1, 88.2 and 96 kHz and at buffer sizes 32, 128,
   256, 512 and 1024.
6. Inspect the Cubase real-time audio thread. Compare self-time in
   `SpringModule::process`, `juce::dsp::Convolution::process`,
   `ChorusModule::process`, `EchoModule::process` and `TapeModule::process`.
7. Save the `.trace` file and note average/maximum Cubase Audio Performance
   values for each pass.

## Host reliability matrix

- Cubase: live monitoring, automation, preset stepping and offline export.
- Ableton: 1, 4, 8 and 16 instances; rapid device bypass; freeze/export.
- Standalone: Input 1, Input 2 and both; mono input to stereo output.
- AU: run `auval -v aufx Rkzr Nath`.
- VST3: rescan, reload a saved project and compare recalled parameters.

For every host confirm silence stays silent, bypassed modules lower CPU, left
and right remain balanced, presets do not click, Echo cannot run away, and no
automation movement produces NaN or infinity.
