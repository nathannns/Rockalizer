# Rockalizer V1 Starter

This is checkpoint 1 of the private Rockalizer learning project.

It builds:

- Audio Unit (AU)
- VST3
- Standalone application

Checkpoint v0.2 adds working Input, Low Cut, Hi Cut and Output processing.
Checkpoint v0.3 adds the first working Chorus module with Rate, Depth, Width,
Tone, Mix and smoothly blended bypass.
Checkpoint v0.4 adds the complete tape Echo: Straight, Bounce, Gallop, Cluster
and Wash patterns, free time, host-tempo sync, repeats, tone, wobble, drive and mix.
Checkpoint v0.5 adds dual Tape modes: polished Studio reel-to-reel character
and compressed, unstable Cassette/Portastudio-style character.
Checkpoint v0.6 embeds seven supplied spring IRs and adds Spring Type, Decay,
Dwell, Tone, Drip, Mix and smooth bypass.
Checkpoint v0.7 adds a click-free global power/bypass control, live input and
output peak meters, clip indication, and a preserved dry path around the chain.
Checkpoint v0.7.1 makes the header POWER control explicitly host-automatable
and guarantees that it remains the topmost clickable UI component.
Checkpoint v0.8 adds eight complete factory presets, previous/next navigation,
an editable preset-name field, and persistent user preset saving/loading.

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
