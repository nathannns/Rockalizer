# Rockalizer

This is checkpoint 1 of the private Rockalizer learning project.

It builds:

- Audio Unit (AU)
- VST3
- Standalone application

## Next work / known issues

Rockalizer-relevant subset of the backlog (full list is in Threadline's
`README.md` under "Issues / next work"; numbers match that list — completed
items are removed, not renumbered). Echo UI tidy-up, UI consistency, and
the Options-button click-outside-to-close (former #8, #10, #13) shipped
2026-08-20.

No numbered Rockalizer issue remains open. Backlog #9 was reproduced and
closed on 2026-08-23.

The cross-project effect audit completed on 2026-08-21. Rockalizer's full DSP
matrix passes across supported sample rates, block sizes, mono/stereo paths,
modulation modes and feedback effects. It also removed a real data race between
Spring's asynchronous IR loader and the audio thread.

The later long-run soak reproduced backlog #9 after 53.8 seconds. When the
modulated four-point Hermite tail read crossed the circular buffer's zero
point, adding the buffer length could round a tiny negative `float` position
to exactly `bufferSize`. The integer tap wrapped correctly, but the fractional
position became 3604 instead of 0 and generated one multi-million-level sample.
The read position now wraps its upper bound too. A two-minute accelerated soak
with hundreds of asynchronous tank changes completes at 0.376 peak and 0.0187
maximum sample step. The shared ADAA secants also use double precision and
enforce their nonlinearities' exact mathematical ranges as additional
long-feedback-loop protection.

The follow-up connection audit also moved Tape oversampling latency
notifications out of `processBlock()` and onto the message thread. The audio
thread now only publishes an atomic request, avoiding a host callback during
live processing while retaining correct PDC updates.

Checkpoint v0.2 adds working Input, Low Cut, Hi Cut and Output processing.

The current Tape engine adds a machine-grounded complementary record/reproduce
EQ pair around its magnetic stage. Studio uses the Studer A800 15 ips NAB
3180/50 us landmarks; Cassette follows the Tascam 244's separate record,
bias-oscillator and playback architecture with a 120 us cassette turnover.
The pair is neutral at small signal while record pre-emphasis changes how hard
high frequencies drive magnetic saturation.
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

Checkpoint v0.37 completes the planned feature set. It replaces the duplicate
Guitar Room factory entry with Tight Guitar Room and adds four more adventurous
factory sounds: Purple Motion, Neon Slap, Tape Mirage and Doubled Glass. These
presets deliberately explore the different echo patterns and spring models.
Chorus gains a single press-and-play FLANGER switch with a short click-free
transition. A lightweight stereo DOUBLER beside HI CUT creates two humanised
copies with different short offsets; new note/chord transients gently refresh
those offsets while smoothing prevents delay-tap clicks. Its DOUBLER wordmark
is also its bypass switch and darkens when off. Both new effects are
host-automatable and stored in presets. From this version onward, development
focuses on presets, DSP refinement, CPU efficiency and reliability rather than
adding more controls.

Checkpoint v0.38 moves FLANGER into the top control area of the Chorus pedal
and gives it a compact illuminated-switch treatment. The Doubler is rebuilt as
a stable ADT-style stereo thickener: its left and right copies use fixed,
different micro-delay offsets, while the knob controls only their level. It no
longer re-randomizes delay time on transients and contains no cyclic modulation,
removing the audible pitch wobble while also reducing CPU work.

Checkpoint v0.39 redesigns the one-button Flanger from published tape-flanging
principles: a much shorter swept delay, stronger negative feedback and a near-
equal dry/delayed balance create clearly audible moving comb-filter notches.
Its new amber hardware LED switch sits below the Chorus knobs and above the
wordmark. Doubler offsets are shortened to 6.2 ms and 9.4 ms so they thicken
instead of reading as slap delay. NOISE GATE is now a clickable bypass wordmark.
All creative modules are skipped at processor level while bypassed and reset
once on shutdown, including convolution and feedback memory, for lower idle CPU.
The five footer controls are grouped more tightly with exactly equal spacing.

Checkpoint v0.40 makes the neutral signal path genuinely transparent: unity
input/output gain and the 20 Hz/20 kHz filter endpoints no longer run DSP, and
the global dry/wet loop is skipped once fully on. This removes the last always-
active phase-rotating processing from an all-off setup. Input/output meters are
30% longer. FLANGER is larger, while Echo SYNC now uses the same generated amber
LED interaction and styling. The tighter footer retains equal 215 px spacing.
Fresh instances and Clean Studio now start with Noise Gate and Doubler bypassed;
factory presets that intentionally use Doubler explicitly enable it.

Checkpoint v0.41 is optimization-only. Spring uses capped 48 kHz stereo IR
assets and non-uniform convolution, skips zero Dwell/Drip work, and retains its
quantized decay cache. Chorus replaces trigonometric triangle generation and
reuses stereo LFO values. Echo caches tone/sync calculations and skips zero
Wobble/Drive work. Tape bypasses neutral processing and skips zero compression
and Age transport work. Raw parameter pointers are cached at construction.
Delay-memory resets now use validity counters instead of clearing large buffers
on the audio thread. Preset recall and Echo bypass fade before invalidating the
tail, fixing the exposed high-Mix click. A Release DSP matrix and Instruments
procedure are included in `PROFILING_V0.41.md`. Input/output meters are 25%
longer. Checkpoint v0.42 removes Echo's phase-offset stereo wobble and upgrades
its modulated reads to four-point interpolation, preventing alternating L/R
clicks at high Mix. Tape Drive keeps its v0.41 response and adds a restrained
fuzzy magnetic-overload edge only above roughly 78%. The Tape type, Flanger,
Echo type, Sync and Spring type controls now share one aligned 112 x 32 row.

Checkpoint v0.43 places a one-sample median de-clicker before the Echo output
and feedback path, eliminating isolated impulses without low-pass filtering the
repeats. Doubler becomes a true dual-voice ADT treatment with unequal short
delays, higher copy level and independent slow micro-pitch drift. Flanger gains
deeper swept delay, stronger bounded resonance and a wetter output balance.
FLANGER and SYNC have larger LED artwork and lettering.

Checkpoint v0.44 removes the temporary Echo de-clicker and corrects the root
cause: the non-integer flutter multiplier jumped whenever the slow LFO wrapped.
Slow wobble and flutter now use independent continuous phases. Flanger has a
faster 0.65–2.5 Hz range and a mostly shared stereo sweep, reducing auto-pan.
LOW CUT and HI CUT sit closer together, and the full-length meters no longer
touch neighboring knobs.

Checkpoint v0.45 keeps the v0.44 Echo implementation unchanged. Flanger has a
lower wet floor, less resonance and more direct level to preserve midrange.
Doubler's dual ADT voices move to 8.5/15.5 ms and gain more level for an obvious
but still non-slap widening effect. A taller footer places DOUBLER beneath its
knob. Tape's upper Drive range uses a slightly smaller fuzz blend and adds
2.7–3.1 dB of progressive post-tape lift at the extreme endpoint.

Checkpoint v0.46 keeps Echo unchanged and rebuilds the footer as six equal-size
controls, all labeled above their knobs. Flanger preserves more direct body and
reduces its stereo phase separation/crossfeed again. Five new factory presets—
Chrome Funk, Beef Tape, Dirty Dimension, Purple Jet and Broken Cassette—expand
the gritty, beefy and Flanger-focused side of Rockalizer.

Checkpoint v0.47 is a layout-only refinement. The bottom rack plate is shorter
and lower, every knob is contained within it, and the meters move into the row:
Input meter between Noise Gate/Input and Output meter between Doubler/Output.
Echo Type shifts slightly toward Sync. Audio processing is unchanged.

Checkpoint v0.48 refines the header and performance controls. The Rockalizer
logo is larger and shifted right, the bottom-row knobs are vertically centred,
and New/Save/Delete use compact plus, disk and trash icons. Delete is available
only for user presets. Flanger now has two mutually exclusive hardware-style
buttons: Mode I is warmer and restrained, while Mode II is faster, deeper and
more aggressive. Older presets using the original Flanger switch load as Mode I.

Checkpoint v0.49 introduces dedicated transparent Flanger hardware artwork with
a visible FLANGER wordmark and interactive Mode I/II buttons. It also shifts the
Rockalizer logo farther right, adds a larger gap above the shorter bottom plate,
centres its knobs lower inside the plate, and slightly strengthens Chorus at low
Mix values without altering the established Echo processing.

Checkpoint v0.50 fixes the Flanger artwork paint order, moves the Rockalizer
logo farther right, and makes the two Flanger switches combinable. Button I is
Mode I, button II is Mode II, and illuminating both activates the wetter,
faster and more aggressive Mode III. Echo processing remains unchanged.

Checkpoint v0.51 removes the rectangular Flanger-state shadow by limiting state
shading to the indicator lamps. Spring Mix now follows an insert-oriented blend:
the wet tail grows into a denser wash while a controlled direct-signal floor
preserves guitar body and attack at very high Mix settings.

Checkpoint v0.52 aligns Flanger state shading with the real artwork lamps and
softens the strip into the Chorus pedal. Spring gains a subtle cross-coupled
mechanical tail for a less obvious fade. Global bypass now visually darkens all
four complete pedals. Doubler preserves the original stereo signal and uses its
single Amount control as a macro for added-voice level, separation and subtle
timing/pitch variation. The Rockalizer logo moves down and right.

Checkpoint v0.53 removes the troublesome Flanger strip artwork completely.
Modes I and II now use the same code-rendered LED control as Echo Sync, while
pressing both still selects the stronger Mode III. Doubler is replaced by a
single-control Fender-inspired bias Tremolo: a warm rounded pulse at a classic
amp speed, with a retained dry-level floor and identical modulation on left and
right so it cannot autopan. Tremolo defaults to bypassed.

Checkpoint v0.54 slows the Fender-style Tremolo to 3.2 Hz, tightens the
FLANGER wordmark against its Mode buttons, and makes ComboBox and LED-button
typography respond to smaller editor sizes. The complete factory bank is
retuned for the latest DSP, including explicit Flanger modes and more restrained
Echo/Spring levels. Midnight Tremolo, Brownface Pulse and Tremolo Dream add
three purpose-built Tremolo presets.

Checkpoint v0.55 gives the Flanger wordmark breathing room and recalibrates
Chorus/Flanger stereo modulation for stronger mono compatibility: Chorus uses
a controlled 129.6-degree stereo phase relationship while Flanger converges
near a shared sweep with reduced pitch excursion. Tape adds Off, 2x and 4x
oversampling quality choices, defaulting to 2x. Spring now feeds Dwell and
transient-sensitive Drip into the convolution input, applies a smooth taper to
shortened IRs, and uses a quieter damped mechanical bridge for a softer decay.

Checkpoint v0.56 moves Tape oversampling into Options and makes the Rockalizer
logo open a dedicated About overlay with its own close control. Chorus becomes
a stronger three-voice ensemble: staggered Dimension-style taps, gentle
BBD/compander rounding inspired by CE-1 character, a more immediate JUNO-like
mix taper, and substantially greater wet authority at 100%. Spring follows with
a slower input-aware release, allowing the convolved tail to bloom behind the
modulated guitar while preserving the initial pick attack.

Checkpoint v0.56.1 fixes the JUCE close-icon line construction that prevented
v0.56 from compiling on macOS and removes the new Tape channel-count warnings.

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

## v0.56.2 build hotfix

This checkpoint fixes the ambiguous JUCE `drawText` overload in the About
panel by explicitly using floating-point bounds, and cleans nearby conversions.

## v0.57.0 DSP refinement

- Keeps the original Rockalizer artwork but makes its About hit target fully
  invisible, including hover, pressed, and keyboard-focus states.
- Smooths the chorus modulation with continuous sine-derived voices, retunes
  the three delay taps, reduces metallic feedback, and strengthens the warm
  stereo ensemble balance.
- Adds restrained low-mid and bass body as Tape Tone is turned below neutral.
- Caches Chorus filter coefficients and its fixed crossover coefficient so
  unchanged parameters do not repeat unnecessary exponential calculations.
- Caches unchanged Echo tone and Spring tone coefficients, plus Spring envelope
  timing constants, reducing repeated work in normal real-time processing.

## v0.58.0 lush chorus refinement

- Expands Chorus mode from three to four decorrelated delay voices.
- Adds slow secondary motion for a denser ensemble without obvious vibrato.
- Removes Chorus-mode feedback to prevent metallic ringing on sustained notes;
  Flanger feedback remains unchanged.
- Adds restrained BBD/compander rounding and low-mid warmth.
- Rebalances dry and wet energy so high Mix settings remain smoother and more
  even instead of pulsing or thinning the guitar.

## v0.59.0 spring, preset, and depth update

- Fixes saved user presets selecting `-- INIT --` by resolving the written
  user file case-insensitively and never accepting an invalid preset index.
- Saving a factory preset name now creates a uniquely named user copy.
- Raises the Spring dry floor at high Mix, reduces transient ducking, and adds
  a denser, softly damped late tail so bloom increases without losing the note.
- Deepens Chorus Depth while leaving Flanger depth calibration intact.
- Revoices all factory presets for the upgraded Spring Mix range, including
  several deliberately wetter ambient sounds.

## v0.60.0 spring architecture and preset editing

- Saving an edited factory preset now updates a hidden user override under the
  same name and preset position; it no longer creates a duplicate. User presets
  continue to overwrite their own XML file. `-- INIT --` remains immutable.
- Rebuilds Spring as a hybrid processor: the selected IR supplies its physical
  onset and resonances, followed by a damped four-line feedback matrix for a
  smoother, denser two-stage bloom at high Mix and Decay settings.
- Retains only three deliberately distinct tanks: 201, 9100, and Tape Mixer.
  British, Deluxe, German, and Hi-Fi and their embedded WAV assets are removed.
- Replaces Tape and Spring dropdowns with immediately visible segmented model
  selectors using the same condensed uppercase style as FLANGER and SYNC.
- Adds a global +1.8 dB output calibration after the effects and filters. This
  applies consistently to every preset while the Output knob remains at 0 dB.

## v0.61.0 integrated selectors and spatial refinement

- Replaces the boxed orange Tape and Spring selectors with the same unboxed
  LED-and-word treatment used by FLANGER and SYNC.
- Places Tape model, Flanger modes, Echo type/Sync, and Spring model controls
  on one shared horizontal baseline.
- Refines Chorus around Dimension-style behavior: four shallow staggered BBD
  taps, slower multi-phase motion, restrained compressor/expander dynamics,
  warmer filtering, steadier dry anchoring, and more controlled stereo
  crossmix for lush width without obvious pitch wobble.
- Adds extremely slow sub-millisecond modulation to the Spring late-field
  delay network. This disperses fixed resonances and produces a smoother,
  denser bloom while leaving the physical IR attack and tank identity intact.

## v0.62.0 selector and preset organisation

- Rebuilds the FLANGER wordmark with the same upright bold typography and
  vertical alignment as its two mode controls.
- Removes LEDs from Tape and Spring model selectors, leaving compact unboxed
  text states that remain readable at the minimum editor size.
- Renames the Tape Mixer spring model to Tape and redistributes all three
  Spring model hit targets evenly across the pedal.
- Moves both footer meters downward for clearer separation from their knobs.
- Separates the preset menu into Factory Presets and User Presets sections
  while preserving every existing preset ID, saved-project reference, and
  factory override.

## v0.63.0 model grouping and high-mix spring wash

- Tightens the model labels into two deliberate control groups and adds visible
  separators: `STUDIO | CASSETTE` and `201 | 9100 | TAPE`.
- Uses balanced label widths and gaps so both groups remain centred and legible
  throughout the editor's supported resize range.
- Preserves Spring's direct-signal clarity through normal insert Mix settings,
  then applies a nonlinear transition above 65%: dry level falls further while
  wet level rises slightly, producing a genuinely washy high-Mix sound without
  abruptly losing the guitar.

## v0.65.0 softer spring tail and selector spacing

- Extended the measured spring onset and its taper so it blends smoothly into the late decay field.
- Increased the late-field overlap at longer decay settings to prevent an audible abrupt ending.
- Moved `STUDIO` and `201` inward toward their separators for more balanced model selectors.

## v0.64.0 multi-spring tank architecture

- Keeps a short, fixed physical onset from each selected IR instead of
  rebuilding and shortening a multi-second convolution whenever Decay moves.
- Moves Decay into a continuously calculated 0.8–10 second feedback tank,
  eliminating quantised IR-length changes and producing a smoother tail.
- Adds model-specific interacting dispersive paths: two darker springs for
  201, three balanced springs for 9100, and two uneven character springs for
  Tape. These feed the modulated four-line late field for increased complexity.
- Integrates Tone into both output filtering and late-field damping, while
  Drip now also excites the dispersive spring network.
- Re-centres `STUDIO | CASSETTE` and `201 | 9100 | TAPE` using balanced visual
  whitespace on both sides of every separator.

## v0.66.0 shared-module cleanup, real bypass, antialiasing, and a Tape fizz fix

- **Historical global bypass fix.** This version corrected where the dry
  crossfade buffer was captured. A later unified-power update superseded the
  dry-bypass behavior: Global Off now fades to silence and skips all effects,
  matching Threadline's combined global-off/mute control.
- **Noise gate deduplicated.** The ~80-line inline three-band hysteresis gate
  in `processBlock` is replaced by `NoiseGateModule` (ported from Threadline,
  identical math), removing a maintained-twice duplicate between the two
  plugins.
- **Doubler wired up.** `DoublerModule` and its `doubler`/`doublerOn`
  parameters existed but were never instantiated in the signal chain or
  exposed in the UI — a whole advertised effect was silently dead. Runs
  first in the chain, ahead of Tape (widens the raw input into a detuned
  stereo pair before Tape's saturation/hysteresis and everything after it
  colors that already-doubled signal, rather than doubling an already-driven
  mono source), with a footer knob + bypass button placed between Hi Cut and
  Tremolo for layout reasons — that placement is independent of where it
  actually runs in the signal path.
- **Tremolo gets a Rate control.** Every other module exposed 4-6 params;
  Tremolo's rate was hardcoded at 3.20Hz. Now a real `tremoloRate` parameter
  (0.5-10Hz), sharing the footer slot with Amount via two narrower knobs.
- **Tape's directional hysteresis no longer sounds fizzy.** The direction
  signal driving the hysteresis curve's bias shift was an instantaneous,
  per-sample hard sign flip on the driven signal's derivative — on a
  harmonic-rich, heavily-driven signal this flips far more often than the
  fundamental's own rate (measured ~1450 flips/sec on a 220Hz test signal
  with a small added wiggle, versus ~440/sec expected), injecting broadband,
  alias-prone energy that got worse exactly as Drive increased (both the
  jump size and the wiggle rate scale with it together) — this is what read
  as "fizzy" distortion/fuzz. Fixed by lowpassing the raw ±1 sign itself
  (500Hz corner) rather than using it directly: a fast, low-amplitude wiggle
  that flips back and forth faster than the filter can track just averages
  toward zero, while a genuine sustained half-cycle of playing still settles
  close to ±1 well within it (verified in a standalone harness: >99% of full
  swing preserved up to ~330Hz, ~90% at 660Hz, while cutting the noisy
  test signal's spurious direction-flip energy by roughly 40%).
- **Chorus/Echo/Spring feedback nonlinearities antialiased.** Same aliasing
  class Klon/TS9/Amp already oversample to avoid: Chorus's BBD input
  saturation and Chorus-mode rounding stage, Echo's feedback-drive
  saturation and write-side safety rail, and Spring's per-spring dispersion
  drive and 4-line tail FDN write (feedback up to ~0.97 at long Decay) were
  all evaluating `tanh()` directly at base rate inside feedback loops that
  compound whatever aliasing that generates on every repeat/reflection.
  Oversampling the whole delay/FDN lines to fix this would need their
  buffer indexing to track an oversampled rate too, so `Antialiasing.h`
  (ported from Threadline) uses antiderivative antialiasing (ADAA) instead
  — the exact average slope of the nonlinearity's antiderivative between
  consecutive samples, mathematically equivalent to bandlimiting its output
  at zero added latency, with a direct-evaluation fallback when consecutive
  samples are too close together for that secant to stay well-conditioned.
- **Chorus and Spring's tail network now use 4-point Hermite interpolation**
  instead of 2-point linear, matching Echo's own `readDelay` (whose header
  comment already explains why: linear interpolation's slope discontinuities
  are audible as clicks on modulated repeats) — Chorus's whole character
  rides on a smooth ~0.2-2.3ms delay-time sweep, making it if anything more
  exposed to this than Echo was.
