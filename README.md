# Rockalizer V1 Starter

This is checkpoint 1 of the private Rockalizer learning project.

It builds:

- Audio Unit (AU)
- VST3
- Standalone application

The audio path is intentionally transparent. The interface is a functional
placeholder matching the agreed four-module structure. DSP and interactive
controls are added in later checkpoints.

## macOS build

From Terminal, open this folder and run:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The first configure downloads JUCE 8.0.15, so it takes longer than later builds.

After building, launch the standalone version:

```bash
open build/Rockalizer_artefacts/Debug/Standalone/Rockalizer.app
```

The AU and VST3 are copied to the normal user plug-in locations after a
successful build.

## Validate the AU

```bash
auval -v aufx Rkzr Nath
```

## Cubase

Open Cubase, rescan VST3 plug-ins if necessary, insert Rockalizer on an audio
track, and confirm that audio passes through unchanged.
