# BDFR Body Authoring — Native Daz Studio Plugin (v0.5.0)

This directory is the native C++/Qt replacement for the early `BDFR_BodyAuthoring.dsa` prototype.

The plugin follows the architectural direction used by the official Daz Bridges: native Daz SDK actions/dialogs and Qt widgets are used for the production UI, while Daz Script remains useful for launchers, automation and QA.

## Current native milestone

Implemented in source:

- responsive three-panel editor layout matching the approved BDFR concept
- Female/Male Genesis body viewers using the user-approved silhouettes
- vector Genesis-style skeleton overlay and optional joint labels
- clickable anatomical presets for breast/pectoral, biceps, abdomen, glute, thigh/quadriceps and calf
- active Female/Male viewer state with pink/blue visual language
- quick region type and left/right/both symmetry controls
- real region list, add/remove/clear and create-from-Daz-selection actions
- selected-region inspector
- radius X/Y/Z controls
- mass, stiffness, damping, compliance, activation, bulge and collision controls
- Walk/Run/custom animation metadata table
- Daz Timeline Play/Pause control
- BDFR body profile JSON export using `DzJsonWriter`
- Genesis 8/8.1/9 anchor-name presets aligned with the CharacterBridge skeleton adapter

Back/Side body images are intentionally disabled until approved source maps exist; the Front viewer is functional rather than presenting fake views.

## Build requirements

- Daz Studio SDK installed through DIM
- Daz Studio 4.x (current test target: Daz Studio 4.23)
- CMake
- Visual Studio on Windows, or the Daz-supported toolchain on macOS
- the Qt 4.8.1 libraries bundled with the Daz SDK — **do not link an unrelated system Qt build**

Example Windows configuration:

```powershell
cmake -S . -B build `
  -DDAZ_SDK_DIR="C:/Users/Public/Documents/My DAZ 3D Library/DAZStudio4.5+ SDK" `
  -DDAZ_STUDIO_EXE_DIR="C:/Program Files/DAZ 3D/DAZStudio4"
cmake --build build --config Release
```

If `DAZ_STUDIO_EXE_DIR` is provided, the compiled plugin is emitted into the Daz `plugins` folder.

## Relationship to DazBridgeUtils / DazToUnreal

The native rewrite was designed after reviewing the official Apache-2.0 `DazBridgeUtils` and `DazToUnreal` architecture:

- native Qt/Daz SDK UI instead of a large scripted dialog
- action/UI separation
- `DzJsonWriter` metadata output
- future DTU integration on the Daz export side
- Daz Script retained for QA/automation rather than production layout

This milestone does **not** copy Daz Bridge source files into BDFR. A later integration layer can optionally link to or extend DazBridgeUtils and append `BDFRBodyProfile` directly to DTU export.

## Current limitation

The source is implemented for this milestone but cannot be compiled in the ChatGPT runtime because the proprietary Daz Studio SDK is not installed here. The next validation step is a local SDK build against Daz Studio 4.23; any SDK-specific compile errors should be fixed before calling the native plugin beta/stable.
