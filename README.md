# Rockalizer

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
Checkpoint v0.9 smooths echo feedback, tone, wobble and drive changes, lowers
the maximum feedback margin, removes known conversion warnings, and adds NaN,
infinity and runaway-level protection around the complete processing chain.
Checkpoint v0.10 adds an explicit NEW preset workflow and turns the four module
titles into their bypass controls, with darkened titles when modules are off.
Checkpoint v0.10.1 enables macOS audio-input permission for the Standalone app.
Checkpoint v0.10.2 adds independent Input 1 and Input 2 switches. Either input
or both can be enabled; selected inputs are centred through both output channels.
Checkpoint v0.10.3 skips fully bypassed DSP modules and the complete chain when
global bypass is settled, adds mono input routing, and makes Tape wow subtle
and mono-compatible instead of producing left-right movement. Production use
should always use a Release build.
Checkpoint v0.10.4 replaces the single input selector with independent Input 1
and Input 2 switches. Any enabled interface input is summed safely and written
to both channels before the effects, preventing mono guitar from appearing only
on the right output.
Checkpoint v0.11 makes Tape Drive more responsive, keeps interface input routing
unchanged when presets are loaded, groups routing and view controls under
OPTIONS, improves Tape/Spring type placement, and adds a beginner-friendly
Simple view with an ADVANCED switch for specialist controls.
Checkpoint v0.12 replaces the preset ComboBox's automatic positioning with a
menu anchored directly below the preset field, and substantially increases the
Tape Drive range and low-control sensitivity.
Checkpoint v0.13 moves the complete preset section left to separate it from
OPTIONS and introduces an embedded, rotating dark-metal knob image across the
plug-in interface.
Checkpoint v0.14 removes generated Tape hiss, adds a persistent linked-channel
Noise Cut gate before the effect chain, fixes the knob image's RGBA rendering,
and changes Tape Compression to a smooth one-knob pedal-style response with a
progressive threshold, ratio, soft knee, and makeup gain.
Checkpoint v0.15 prevents Tape Drive and compressor makeup from raising the idle
interface floor, moves the NOISE CUT label below its knob, and introduces
textured pedal panels: brown Tape, green Chorus, purple Echo, and light-blue
Spring.
Checkpoint v0.16 embeds cleaned copies of all seven Spring IRs to reduce recorded
tail noise—especially Pioneer and Space—and makes the pedal texture more visible
with stronger material grain, enclosure depth, highlight borders, and bevels.
Simple mode now uses the same three-knob horizontal grid on every pedal, while
Advanced mode uses aligned two-column rows.

For guitar, Rockalizer expects an instrument/Hi-Z interface input at a healthy
level. Interface Line/Instrument impedance is selected in hardware or interface
control software before audio reaches the plug-in and cannot be changed by DSP.

Checkpoint v0.17 locks all Simple-mode knobs to their permanent top-row
positions and only reveals extra controls underneath in Advanced mode. It also
gives Echo SYNC a full-width row, lowers Noise Cut, expands the footer around
all value boxes, and adds an original dark, hazy 1990s R&B-inspired background.

Checkpoint v0.18 moves OPTIONS into a hamburger-controlled floating panel and
adds persistent Line/Instrument digital calibration. Pedal colours are retuned
to the background palette, and Tape Drive now increases the waveshaper strength
and harmonic asymmetry—not only the signal level feeding it.

Checkpoint v0.19 keeps the existing Noise Cut processing unchanged, makes the
first part of Tape Drive slightly cleaner, puts ADVANCED beside a code-drawn
OPTIONS gear, lowers the Tape and Spring selectors, and introduces an original
late-80s/90s Rockalizer wordmark. Borderless condensed module names and unique
reel, wave, echo-ripple, and spring-coil enclosure motifs give each pedal its
own identity while keeping the four modules visually cohesive.
New instances now select Input 2 by default; routing remains persistent and is
still excluded from preset changes.

Checkpoint v0.20 replaces the procedural pedal cards with four separate
original 3D enclosure images and enlarges the Rockalizer wordmark on a raised
gradient plate. Module names now use a matching shadowed late-80s/90s display
treatment. Tape COMP follows a linked one-knob pedal curve with progressively
lower threshold, higher ratio, longer sustain, makeup gain, and mild harmonic
colour. AGE now audibly combines high-frequency wear, head bump, modulation,
and saturation; bright Tone settings retain more midrange body. Echo Wobble has
a clearer but controlled slow-wobble/flutter contour. Chorus now uses a custom
dual-phase, two-tap stereo modulation design for a subtle Dimension-style width
rather than a generic obvious chorus sweep.

Checkpoint v0.21 enlarges the Rockalizer wordmark and removes its logo plate in favour of a clean drop
shadow, crops each rendered enclosure to fill its complete module boundary,
and replaces the decorative module titles with restrained bold Futura-style
90s typography. Tape COMP is recalibrated so clockwise movement unmistakably
adds makeup gain and increasingly drives the tape waveshaper instead of becoming
quieter under heavy gain reduction. The large POWER text control is replaced by
a compact, subtle standby icon that dims when bypassed and glows softly when on.
The preset strip is shifted right to give Noise Cut a clearer visual boundary.

Checkpoint v0.22 makes the Tape signal order explicit: Noise Cut first, then
the complete one-knob compressor, then tape transport, drive, tone and age.
Compressor makeup now hits the tape input rather than being applied inside the
tape colour stage, so increasing COMP creates more sustain, level and tape
harmonics while Tape Mix still blends against the original gated signal.

Checkpoint v0.22.1 fixes the JUCE 8 pedal-image crop call by using the supported
destination/source coordinate overload, allowing PluginEditor.cpp to compile.

Checkpoint v0.23 reduces compression-driven saturation substantially while
keeping clockwise makeup gain, defaults COMP to 20% and AGE to 0%, and restores
filtered pre-saturation bass and midrange as Tape Drive rises. Four new clean
RGBA pedal faces preserve the established brown, teal, purple and blue materials
but remove every decorative object behind the controls; their genuine alpha
edges reveal the plug-in background and retain only natural enclosure shadows.
The larger Rockalizer logo, Noise Cut, preset strip, Advanced, Options and Power
controls now share one header centre line.

Checkpoint v0.24 removes the extra outlines around the pedal artwork and keeps
the original Chorus, Echo and Spring images visually solid against the dark
background. Header controls share the preset strip height and the logo is
larger. Tape Drive now follows picking dynamics, with quieter notes staying
cleaner and harder notes reaching progressively more saturation. COMP is the
final Tape stage before Chorus; its high-pass-weighted detector preserves bass
body and a subtle parallel low band adds thump. Echo feedback saturation is
unity-normalised to prevent Drive from increasing loop gain, and the Space Echo
factory preset uses safer Repeats and Drive defaults.

Checkpoint v0.25 removes all added colour beds from behind the pedal images,
leaving only the original transparent artwork and shadows. Noise Cut is placed
above a clearly separated label. Tape Drive's playing threshold is lowered for
instrument-level guitar. Echo Sync moves note divisions onto eight fixed Time
knob positions and removes the separate division selector. Spring DECAY now
truncates the embedded IR at stepped physical lengths, DWELL uses unity-slope
transducer saturation, TONE damps the convolved tail, and DRIP is weighted by
input transients.

Checkpoint v0.26 aligns the header into a consistent 56-pixel control row and
adds more breathing room after the shifted Rockalizer logo. Echo Pattern now
matches the centred Tape/Spring selector treatment, with Sync beneath it. TAPE,
CHORUS, ECHO and SPRING use independent transparent wordmark assets which act
as their bypass buttons and dim when off. All pedal knobs are reduced to 68 px
while keeping their fixed Simple/Advanced positions. The Rockalizer wordmark is
embedded as the Standalone/AU/VST3 application icon source.

Checkpoint v0.27 removes the Advanced button border, renames the visible Noise
Cut control to Noise Gate, and applies equal 20-pixel header gaps. Rebuilt v2
effect-title images remove trapped white/checker pixels from enclosed letters.
The four image titles render at a common 34-pixel visual height and sit seven
pixels lower while remaining clickable bypass controls.

Checkpoint v0.28 replaces Advanced with a custom text-only control so no JUCE
button border can reappear. Every effect wordmark is now fixed at the bottom of
its pedal, with Tape/Spring type and Echo Pattern/Sync arranged in a matching
selector row directly above. Tape uses a flatter v3 burgundy enclosure matching
the other three pedals. Drive compensation reduces the level jump between clean
soft notes and saturated hard notes. Saved state now records the selected preset
and explicitly reloads it on open, keeping the preset name and sound in sync.

Checkpoint v0.29 moves both knob rows, the selector row and all four effect
wordmarks upward so they remain fully inside the pedal faces. Every wordmark is
rendered into the same visual box. Bypassing an effect now dims its enclosure,
knobs, labels and settings together with its title. Noise Gate is rebuilt as a
stereo-linked downward expander with three frequency-trimmed detectors,
hysteresis, hold and smooth attack/release. Tape Drive no longer uses an
artificial playing-level switch: a stateful, input-dependent saturation stage
keeps soft signals near-linear while progressively compressing and colouring
harder peaks, with small-signal gain compensation.

Checkpoint v0.30 moves both knob rows another 40 pixels toward the top of each
pedal and raises the selector/title group by 20 pixels. Tape Drive gains a wider
22 dB Studio / 28 dB Cassette record-drive range and stronger saturation shape,
while retaining small-signal level compensation. The Tape wordmark is rebuilt
from its visible pixels into the same 480×72 artwork bounds as Chorus, removing
the internal padding that previously made it render visibly smaller.

Checkpoint v0.31 rebuilds Chorus around a Dimension-style dual-delay topology:
slow shallow triangle modulation, inverted stereo motion, filtered opposite-
polarity cross-mixing, subtle cross-feedback and a steadier dry level create a
lush width without obvious pitch wobble. Chorus defaults and all eight factory
presets are retuned for the new chorus and stronger Tape Drive. Effect wordmarks
now preserve aspect ratio at a common visible letter height. The selector/title
group sits 12 pixels lower, pedal knobs gain a close contact shadow and subtle
bronze mounting rim, and the footer panel uses a translucent fill. Spring types
are displayed as British, Deluxe, 201, 9100, Tape Mixer, German and Hi-Fi.

Checkpoint v0.32 reduces only the Tape wordmark to 84% of the common letter
height, correcting its larger visual weight. Knobs receive an opaque dark body
behind their RGBA artwork so pedal texture cannot bleed through. Spring now
uses an equal-power dry/wet curve, a 115 Hz wet-only high-pass filter, restrained
normalised-IR tail gain and light input-aware wet ducking. Higher Mix settings
retain considerably more note definition while the tail blooms after attacks;
100% Mix remains intentionally wet-only.

Checkpoint v0.33 replaces Echo's hard feedback-buffer rail with a smooth bounded
curve, preventing overloaded wet repeats from exposing click edges at high Mix.
Preset changes now request an audio-thread-safe reset of all time-based memories
at the next block boundary; the modules then use their existing wet ramps to
bring the new preset in without Doppler beeps from the previous Echo tail. Knob
columns are tightened and centred on every pedal. Studio Tape gains 3 dB more
record drive, a broader saturation curve and slightly stronger magnetic memory
for more audible but still level-compensated analog-style colour.

Checkpoint v0.34 makes Clean Studio the deterministic startup sound for every
new instance and adds a selectable -- INIT -- factory preset with all creative
knobs at zero, unity input/output and transparent filter endpoints. Interface
input routing remains outside preset recall. Echo Mix now uses a curved taper
for much finer low-range control: low settings create subtle ambience while the
top of the knob still reaches a fully wet delay.

Checkpoint v0.35 separates the two Tape media more clearly at ordinary guitar
settings. Cassette now keeps an audibly narrower bandwidth, stronger low-mid
body, higher record level, greater bias asymmetry and more magnetic memory even
with AGE at zero, while Studio retains its wider, cleaner high-headroom curve.
Clean Studio remains the factory default for every new plug-in instance; saved
DAW instances continue restoring their chosen sound so existing sessions are
not overwritten.

Checkpoint v0.36 replaces Tape's delayed parallel blend with one calibrated
series record/repro path. MIX now morphs the amount of tape behaviour instead
of placing saturated audio beneath the raw guitar. DRIVE 0 is unity-clean;
increasing Drive progressively lowers magnetic headroom, compresses peaks and
adds level-dependent odd/even harmonics while inverse record-gain compensation
holds nominal loudness. Studio and Cassette use clearly different calibration,
bias, bandwidth and memory curves, and transport displacement occurs only when
AGE is raised.

## macOS build

From Terminal, open this folder and run:

```bash
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

The first configure downloads JUCE 8.0.15, so it takes longer than later builds.

After building, launch the standalone version:

```bash
open build-release/Rockalizer_artefacts/Release/Standalone/Rockalizer.app
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
