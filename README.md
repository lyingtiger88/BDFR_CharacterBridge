# BDFR_CharacterBridge

**BDFR_CharacterBridge** is an open, extensible character pipeline for **Unreal Engine 5.8+** focused on bringing external digital humans into Unreal with a consistent skeleton, animation, facial-expression and metadata workflow.

The project starts with **Daz Studio / Genesis** characters and is designed to expand to Blender, MetaHuman-compatible assets and custom skeletal characters without locking the runtime to a single authoring tool.

> **Project status:** Early development / architecture foundation.  
> Features below marked as planned are roadmap targets and are not yet production-ready.

## Why this project exists

Character transfer is usually much more than importing an FBX. A production-ready bridge needs to preserve or rebuild:

- skeletal hierarchy and reference pose
- bone orientation and twist behavior
- morph targets and corrective shapes
- animation sequences and root motion
- facial expressions and face rig mappings
- IK / retargeting data
- material and texture metadata
- source-character metadata for downstream systems

BDFR_CharacterBridge aims to provide a reusable compatibility layer so the imported character can later connect cleanly to other BDFR systems such as body dynamics, strand/hair simulation, AI and physics.

## Target pipeline

```text
Daz Studio / Blender / Custom FBX / MetaHuman-compatible assets
                            |
                            v
                  BDFR Character Import
                            |
           +----------------+----------------+
           |                |                |
           v                v                v
      Skeleton          Animation          Facial
      Conversion        Transfer           Mapping
           |                |                |
           +----------------+----------------+
                            |
                            v
                  BDFR Character Asset
                            |
                            v
                    Unreal Engine 5.8+
```

## Core goals

### Character import
- Skeletal mesh import pipeline
- Source-profile detection
- Morph-target preservation
- Character metadata and validation
- Extensible adapter interface for multiple DCC sources

### Skeleton conversion
- Daz Genesis 8 / 8.1 / 9 mapping
- Bone-name normalization
- hierarchy mapping
- reference-pose conversion
- twist-bone handling
- scale and axis normalization
- Unreal humanoid / IK-ready output

### Animation transfer
- Animation Sequence transfer
- root-motion preservation
- source-to-target retarget profiles
- IK Rig / IK Retargeter integration
- pose correction
- validation for common locomotion and interaction poses

### Facial pipeline
- Daz facial morph import
- expression mapping
- facial bone support where available
- ARKit-compatible blendshape mapping
- MetaHuman-compatible facial interoperability where practical
- future lip-sync and runtime emotion hooks

## Architecture

```text
BDFR_CharacterBridge
|
+-- Core
|   +-- Character Asset
|   +-- Source Metadata
|   +-- Validation
|
+-- Import
|   +-- Daz Adapter
|   +-- FBX Adapter
|   +-- Blender Adapter          [planned]
|   +-- MetaHuman Adapter       [planned]
|
+-- Skeleton
|   +-- Bone Mapper
|   +-- Reference Pose Converter
|   +-- Twist Resolver
|   +-- Retarget Profile
|
+-- Animation
|   +-- Animation Import
|   +-- Retargeting
|   +-- Pose Correction
|
+-- Facial
|   +-- Morph Registry
|   +-- Expression Mapping
|   +-- ARKit Mapping
|
+-- Editor
    +-- Character Import Wizard
    +-- Validation Report
```

More detail: [Architecture](Docs/Architecture.md)

## Daz to Unreal skeleton strategy

The bridge does not assume that a Daz skeleton should simply be renamed into an Unreal skeleton. Conversion is treated as a structured process:

1. detect the Daz/Genesis source profile
2. capture source hierarchy, transforms and reference pose
3. map semantic bones to the BDFR humanoid schema
4. resolve twist/distribution bones without destroying deformation quality
5. normalize pose, scale and axis conventions
6. generate an Unreal retarget profile
7. validate shoulders, elbows, wrists, hips, knees and feet
8. preserve source-specific corrective data when required

See [Skeleton Conversion](Docs/SkeletonConversion.md).

## Unreal Engine 5.8 direction

The plugin foundation targets Unreal Engine **5.8+** and is being designed around modern Unreal character workflows, including:

- Skeletal Mesh
- IK Rig / IK Retargeter
- Control Rig integration
- Morph Targets
- animation retargeting
- modular editor tooling
- future integration points for ML Deformer / advanced deformation systems

## Repository layout

```text
BDFR_CharacterBridge/
|
+-- Docs/
|   +-- Architecture.md
|   +-- Roadmap.md
|   +-- SkeletonConversion.md
|
+-- Plugins/
    +-- BDFR_CharacterBridge/
        +-- BDFR_CharacterBridge.uplugin
        +-- Source/
            +-- BDFR_CharacterBridge/
                +-- Public/
                +-- Private/
```

## Roadmap

### v0.1 — Foundation
- [x] Repository architecture
- [x] Unreal plugin module foundation
- [x] Skeleton-conversion design
- [ ] BDFR character data model
- [ ] import validation framework

### v0.2 — Daz skeleton adapter
- [ ] Genesis 8 profile
- [ ] Genesis 8.1 profile
- [ ] Genesis 9 profile
- [ ] reference-pose conversion
- [ ] twist-bone resolver
- [ ] automated mapping report

### v0.3 — Animation pipeline
- [ ] animation transfer
- [ ] root motion
- [ ] IK Rig generation
- [ ] IK Retargeter profile
- [ ] pose correction

### v0.4 — Facial pipeline
- [ ] facial morph registry
- [ ] expression transfer
- [ ] ARKit mapping
- [ ] facial validation

### v0.5 — Extended ecosystem
- [ ] Blender adapter
- [ ] MetaHuman interoperability
- [ ] DynamicBodySystem integration
- [ ] StrandFX integration
- [ ] runtime character hooks

See the full [Roadmap](Docs/Roadmap.md).

## Design principles

- **Non-destructive:** retain useful source data instead of flattening everything into one target format.
- **Adapter-based:** Daz support is a first-class adapter, not a hard-coded dependency throughout the project.
- **Retarget-friendly:** skeleton conversion should improve animation interoperability rather than only making an import succeed.
- **Runtime-extensible:** imported characters should be usable by future BDFR simulation systems.
- **Validation-first:** conversion errors should be reported explicitly instead of hidden behind silent approximations.

## Contributing

The project is currently in its foundation stage. Architecture, source-profile definitions, bone-mapping datasets, test characters and retargeting test cases will become increasingly important as implementation progresses.

---

**BDFR_CharacterBridge** — Character interoperability for Unreal Engine.
