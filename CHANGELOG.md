# Changelog

## 0.1.1 - Windows installer

- Added an Inno Setup Windows installer that installs the VST3 bundle to `C:\\Program Files\\Common Files\\VST3`.
- Installer also installs the standalone ClearEQ app and optional desktop shortcut.
- Updated GitHub Actions to build and upload the installer with the Windows test package.

## 0.1.0 - Functional prototype

- Converted the static ClearEQ HTML/image mockup into a real JUCE plugin project.
- Added five functional EQ bands: Low, Low-Mid Mud, Mid, Presence, High.
- Added draggable graph nodes tied to real plugin parameters.
- Added VST3, AU on macOS, and Standalone build targets.
- Added GitHub Actions packaging for Windows, macOS, and Linux test builds.
