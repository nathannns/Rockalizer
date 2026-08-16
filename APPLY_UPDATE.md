# Rockalizer v0.57.0

This package is a complete source snapshot. Replace your current project folder
with this folder, or copy `Source`, `Resources`, `CMakeLists.txt`, and `README.md`
into your existing Rockalizer checkout.

On macOS, build the Release version from Terminal:

```bash
cd ~/Downloads/Rockalizer-v0.57.0
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
open build-release/Rockalizer_artefacts/Release/Standalone/Rockalizer.app
```

Input 2 is the default for new instances. Input routing remains outside preset
recall, so preset changes do not alter the selected interface input.

v0.37 adds the final planned controls: a press-and-play FLANGER switch inside
Chorus and a lightweight stereo DOUBLER beside HI CUT. Click the DOUBLER
wordmark to bypass it; the word and knob darken while off. The duplicated room
preset has been replaced by Tight Guitar Room, and Purple Motion, Neon Slap,
Tape Mirage and Crystal Dimension explore more echo patterns and spring types.

v0.38 places FLANGER neatly at the top of the Chorus pedal. Doubler now uses
stable 12.5 ms and 21 ms stereo copies with no note-triggered random delay and
no cyclic modulation, so it widens and thickens without audible wobble.

v0.39 moves the new amber LED FLANGER switch below the Chorus knobs and makes
the effect substantially more audible. Doubler uses shorter 6.2/9.4 ms offsets
to avoid slapback. Click NOISE GATE to bypass it. Bypassed Tape, Doubler,
Chorus/Flanger, Echo, Spring and Noise Gate now stop their DSP work; time-based
state is cleared once rather than being kept alive in standby.

v0.40 removes neutral endpoint filters and unity gain stages from the active
path, eliminating the remaining always-on processing when the creative modules
are off. Input/output meters are longer, FLANGER is larger, and Echo SYNC now
uses the same amber LED-button interaction.
Noise Gate and Doubler default to bypassed in fresh instances and Clean Studio.

v0.41 is an optimization-only build: lighter Spring IR/convolution work,
cheaper Chorus/Flanger oscillation, cached Echo coefficients and tempo values,
zero-value fast paths in Tape/Echo/Spring, cached raw parameter pointers, and
lightweight delay invalidation instead of large audio-thread buffer clears.
Preset changes and Echo bypass now fade before clearing their tail to prevent
high-Mix clicks. See `PROFILING_V0.41.md` for the automated matrix and exact
Xcode Instruments/Cubase profiling procedure.

v0.42 removes the remaining high-Mix Echo click and apparent auto-pan at its
source: both channels now share one tape-transport wobble and modulated reads
use four-point interpolation without accessing stale delay memory. The existing
Tape Drive response is unchanged below the upper range; above roughly 78% it
gradually adds a restrained fuzzy overload edge. Tape type, Flanger, Echo type,
Sync and Spring type now occupy the same aligned 112 x 32 control row.

v0.43 adds a dedicated median de-clicker before Echo output and feedback, so a
single-sample impulse cannot become an audible repeat even at 100% Mix. Doubler
now uses two louder ADT voices with unequal short delays and independent,
sub-Hz micro-pitch drift. Flanger has deeper sweep, stronger resonant feedback
and a wetter finished level. FLANGER and SYNC use larger LED artwork and type.

v0.44 removes that de-clicker and fixes the actual Echo defect. Flutter had
been calculated by multiplying the slow-wobble phase by a non-integer; whenever
the slow phase wrapped, flutter jumped to a different delay position. Flutter
now has an independent continuous phase accumulator. Flanger runs faster and
uses a much narrower stereo sweep. LOW CUT and HI CUT are closer, while both
meters retain their length with visible gaps before adjacent knobs.

v0.45 leaves the proven v0.44 Echo code untouched. Flanger uses less wet level
and feedback plus a stronger direct path to retain guitar midrange. Doubler's
two voices are louder and use more distinct 8.5/15.5 ms ADT offsets. The footer
is taller and DOUBLER now sits below its knob. High Tape Drive has slightly less
fuzzy distortion but adds a restrained post-tape level lift instead of losing
volume.

v0.46 keeps Echo unchanged. The footer is a uniform six-control strip with
NOISE GATE, INPUT, LOW CUT, HI CUT, DOUBLER and OUTPUT labels above equal-sized
knobs. Flanger retains slightly more direct midrange and uses an almost shared
stereo sweep for less width. Five new factory presets add gritty, beefy and
Flanger-led sounds: Chrome Funk, Beef Tape, Dirty Dimension, Purple Jet and
Broken Cassette.

v0.47 changed the bottom-bar layout. v0.48 enlarges and repositions the logo,
centres the bottom knobs vertically, adds safe user-preset deletion and replaces
the single Flanger switch with mutually exclusive Mode I / Mode II buttons.

The shorter bottom plate sits lower to restore space
below the pedals, with every knob fully inside it. Input metering now sits
between Noise Gate and Input; output metering sits between Doubler and Output.
Echo Type moves slightly right to close the gap to Sync. Echo and all other DSP
remain unchanged.
