# Morph Target Transfer

## Goal

BDFR_CharacterBridge preserves Daz morph targets as first-class character data instead of treating them as anonymous FBX blend shapes.

The transfer path is designed around two complementary inputs:

1. FBX blend-shape channels, which carry the actual mesh deformation;
2. Daz DTU metadata, which carries morph names, labels and joint-driven relationships.

## Current implementation

The Daz morph adapter can now build a `FBDFRMorphTransferPlan` from DTU JSON plus the list of morph targets found on the imported mesh.

The plan preserves:

- Daz internal morph name
- display label
- imported FBX morph name
- intended Unreal target name
- semantic category
- whether the morph is joint-driven
- whether the requested morph exists on the imported mesh
- unlisted but present imported morphs

## Name normalization

DazToUnreal currently accounts for two naming behaviors:

- Daz morph identifiers may include a prefix before a period while FBX export keeps only the suffix.
- FBX blend-shape channel names can contain an object namespace separated by `__`.

BDFR normalizes both forms so DTU metadata can be matched reliably to Unreal morph targets.

Example:

```text
Genesis8Female.FBMBodybuilder
            |
            v
FBMBodybuilder

Genesis8Female__eCTRLSmile
            |
            v
eCTRLSmile
```

## Semantic classification

The first classifier recognizes common Daz conventions and descriptive labels:

- `FBM` / `PBM` -> Body
- `FHM` / `PHM` -> Head
- `eCTRL` and common expression labels -> Expression
- viseme / phoneme names -> Viseme
- `pJCM`, JCM-like names and `_dq2lb` variants -> Corrective
- generic `CTRL` -> Control
- everything else -> Unknown

Classification is metadata only. It does not rename or discard a morph.

## Joint-driven corrective morphs

Daz DTU `JointLinks` data is preserved in `FBDFRJointDrivenMorphLink`.

Fields currently retained:

- `Bone`
- `Morph`
- `Axis`
- `Scalar`
- `Alpha`
- optional `Keys` with `Angle` / `Value`

This is enough to reproduce the driver relationship later in a UE 5.8 Control Rig or another runtime corrective solver.

## Non-destructive behavior

The adapter intentionally keeps morphs that exist on the imported mesh even when they are not listed in the DTU morph list.

They are reported as `UnlistedImportedMorphs` and added to the transfer manifest instead of being silently removed.

This differs from a narrow selection-only pipeline and is important for custom characters, third-party corrective shapes and future facial rigs.

## Next editor-side step

The Unreal editor importer should consume the morph transfer plan and:

1. enable FBX morph-target import;
2. verify imported `UMorphTarget` names against the manifest;
3. apply target/display naming without losing the original Daz internal name;
4. persist the manifest on the BDFR character asset;
5. generate JCM Control Rig logic from `JointDrivenLinks`;
6. expose expression/viseme morphs to the facial-animation layer.
