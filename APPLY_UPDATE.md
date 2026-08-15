# Rockalizer v0.33

This package is a complete source snapshot. Replace your current project folder
with this folder, or copy `Source`, `Resources`, `CMakeLists.txt`, and `README.md`
into your existing Rockalizer checkout.

On macOS, build the Release version from Terminal:

```bash
cd ~/Downloads/Rockalizer-v0.33
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
open build-release/Rockalizer_artefacts/Release/Standalone/Rockalizer.app
```

Input 2 is the default for new instances. Input routing remains outside preset
recall, so preset changes do not alter the selected interface input.

v0.33 removes Echo overload clicks with smooth feedback/output safety curves,
clears old time-based memories safely when a preset changes so they cannot
produce a Doppler beep, tightens the knob grid, and adds moderately stronger
Studio Tape record drive, saturation and magnetic memory.
