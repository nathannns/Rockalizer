# Rockalizer v0.36

This package is a complete source snapshot. Replace your current project folder
with this folder, or copy `Source`, `Resources`, `CMakeLists.txt`, and `README.md`
into your existing Rockalizer checkout.

On macOS, build the Release version from Terminal:

```bash
cd ~/Downloads/Rockalizer-v0.36
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
open build-release/Rockalizer_artefacts/Release/Standalone/Rockalizer.app
```

Input 2 is the default for new instances. Input routing remains outside preset
recall, so preset changes do not alter the selected interface input.

v0.36 rebuilds Tape as a calibrated series path. Drive 0 is unity-clean, higher
Drive progressively saturates without acting like volume, and Mix no longer
places a delayed distorted copy underneath the raw guitar. Studio and Cassette
now use distinct headroom, bias, bandwidth and magnetic-memory curves.
