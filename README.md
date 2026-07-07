# ClearEQ

ClearEQ is a real JUCE-based EQ plugin project based on the dark VST UI mockup from the design sketch. It is intended to become a beginner-friendly, fast, musical EQ that still feels familiar to experienced producers.

## What works now

- Real audio processing: five EQ bands using JUCE DSP IIR filters.
- Plugin formats from the same source:
  - Windows: VST3 + Standalone app
  - macOS: VST3 + AU + Standalone app
  - Linux: VST3 + Standalone app
- Large draggable EQ nodes on the frequency graph.
- Delta mode solos the selected band so testers can hear exactly which frequency area they are adjusting.
- Host-automatable parameters for frequency, gain, Q, bypass, and output.
- GitHub Actions builds downloadable test packages for Windows, macOS, and Linux.
- Windows installer builds are included for easier DAW testing across Reaper, FL Studio, Ableton, Studio One, etc.
- macOS builds are universal Apple Silicon + Intel binaries with a 10.15 minimum deployment target, intended to run on modern macOS versions including Sequoia 15.x.

## Bands

| Band | Type | Intended beginner label |
| --- | --- | --- |
| Low | Low shelf | Weight / warmth |
| Low-Mid Mud | Bell | Mud cleanup |
| Mid | Bell | Body / focus |
| Presence | Bell | Clarity / forwardness |
| High | High shelf | Air / brightness |

## Downloading a test build

1. Open the GitHub repository.
2. Go to **Actions**.
3. Open the latest **Build ClearEQ plugin packages** workflow run.
4. Download the artifact for your OS.
5. For Windows DAWs, unzip `ClearEQ-Windows-VST3-Standalone.zip` and copy `ClearEQ.vst3` to:

```text
C:\Program Files\Common Files\VST3
```

Then rescan plugins in FL Studio, Ableton, Reaper, Studio One, etc.

## Local build

Requires CMake 3.22+, a native C++ compiler, and the usual JUCE system dependencies.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The plugin outputs are under the build directory in `ClearEQ_artefacts`.

## Status

This is an early functional prototype, not a polished commercial plugin yet. Next steps are DAW testing, parameter smoothing, presets, better metering/analyzer support, installer packaging, and signed/notarized releases.
