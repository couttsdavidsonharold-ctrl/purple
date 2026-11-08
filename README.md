# PURPLE by Hdax — CHOP + SCREW (JUCE / C++)

## What it does (v0.1)
- **SCREW** — real-time pitch drop, 0 to -12 semitones (two-tap crossfaded delay line), plus **MURK** (low-pass darkening).
- **CHOP** — tempo-synced rhythmic gate: 6 note divisions (1/4 … 1/16T), 8 step patterns, depth + edge smoothing.
  Follows host BPM and play position (PPQ); free-runs at host BPM (120 if unknown) when the transport is stopped.
- **MIX**, resizable UI, purple "juice" animation driven by the output level (UI thread only; audio thread just writes two atomics).
- Mono and stereo, all parameters automatable, state saved with the session.
- Reports 30 ms of latency to the host (inherent to the pitch shifter) and delays the dry signal to match.

## How to get the real plugin files WITHOUT installing anything locally
1. Create a GitHub repository and upload this whole folder (keep the `.github` folder).
2. Open the **Actions** tab → "Build PURPLE" runs automatically (or press *Run workflow*).
3. When it finishes, download the artifacts **PURPLE-Windows** (`...Setup.exe`) and **PURPLE-macOS** (`...macOS.pkg`).
   The job also runs pluginval (strictness 5) on VST3 and AU; if it fails, the job is red.

## Local build
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## AAX (Pro Tools) — extra requirements, cannot be skipped
1. Free Avid developer account → download the **AAX SDK**.
2. Configure with `-DAAX_SDK_PATH=/path/to/AAX_SDK`.
3. Pro Tools (release) only loads AAX plugins **signed with PACE/Avid tools** (wraptool + your Avid-issued signing account).
   Unsigned AAX only loads in Pro Tools Developer.

## macOS notes
Builds universal (arm64 + x86_64). To let other Macs open the plugins/installer without warnings you need an
Apple Developer ID and notarization (not set up here).

## Status of testing (be honest about what has been verified)
| Item | Status |
|---|---|
| VST3 compiles (Linux, Release) | done |
| pluginval strictness 5, no GUI tests (Linux VST3) | passed |
| Windows / macOS builds, AU, Standalone | configured in CI, **not yet run** |
| AAX | **not built** (needs Avid SDK + signing) |
| Real DAW tests (Pro Tools, Logic, FL, Ableton, Cubase, Studio One, Reaper, Bitwig) | **not done** |
