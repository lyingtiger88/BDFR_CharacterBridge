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
- Skeletal mesh FBX import pipeline
- Source-profile detection
- Automatic Unreal Morph Target import
- Daz morph manifest persisted on the imported Skeletal Mesh
- JCM / joint-driven morph metadata preservation
- Daz-authored muscle / soft-tissue body profiles
- Walk / Run / custom Daz animation test-clip manager
- Add / Remove / Apply / Play-Pause clip controls
- automatic .bdfrbody.json sidecar discovery
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
|   +-- Daz Body Profile Adapter
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
+-- Daz Tools
|   +-- Body Authoring Script
|
+-- Editor
    +-- FBX Character Importer
    +-- Morph Validation
    +-- Character Import Wizard      [planned]
    +-- IK Rig / Retargeter Generator
    +-- Retarget Pose Generator
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

See [Skeleton Conversion](Docs/SkeletonConversion.md), the implemented [Daz Skeleton Adapter](Docs/DazSkeletonAdapter.md), and [Reference Pose Conversion](Docs/ReferencePoseConversion.md).

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
            |   +-- Public/
            |   +-- Private/
            +-- BDFR_CharacterBridgeEditor/
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
- [x] Genesis 8 profile foundation
- [x] Genesis 8.1 source-aware profile foundation
- [x] Genesis 9 profile foundation
- [x] generation detection and validation report
- [x] non-destructive twist-bone preservation policy
- [x] non-destructive reference-pose conversion plan
- [x] component-space pose direction analysis
- [ ] twist rotation/distribution solver
- [x] generated IK Rig / IK Retargeter assets
- [x] generated target Retarget Pose

### v0.3 — Animation pipeline
- [ ] animation transfer
- [ ] root motion
- [x] IK Rig generation
- [x] IK Retargeter generation
- [x] UE 5.8 ChainToChain pose alignment
- [ ] batch animation retargeting
- [ ] locomotion validation

### v0.4 — Morph & facial pipeline
- [x] Daz morph manifest / DTU parsing
- [x] morph-name normalization and semantic classification
- [x] JCM JointLinks metadata preservation
- [x] editor-side FBX morph import integration
- [x] imported morph validation + persistent asset metadata
- [ ] facial expression transfer mapping
- [ ] ARKit mapping
- [ ] facial validation

### v0.5 — Body simulation bridge
- [x] Daz muscle / soft-tissue authoring markers
- [x] .bdfrbody.json export
- [x] Walk / Run test-clip presets and Add/Remove controls
- [x] Daz preset application and playback preview
- [x] body-profile animation-clip metadata transfer
- [x] automatic CharacterBridge body-profile import
- [x] anchor validation against imported skeleton
- [ ] Daz-to-Unreal body-marker coordinate conversion
- [ ] DynamicBodySystem region instantiation
- [ ] muscle activation binding
- [ ] soft-tissue solver binding

### v0.6 — Extended ecosystem
- [ ] Blender adapter
- [ ] MetaHuman interoperability
- [ ] DynamicBodySystem integration
- [ ] StrandFX integration
- [ ] runtime character hooks

See the full [Roadmap](Docs/Roadmap.md), [Morph Target Transfer](Docs/MorphTargetTransfer.md), [Editor FBX Morph Import](Docs/EditorFbxMorphImport.md), [IK Rig / Retargeter Generation](Docs/IKRigRetargeterGeneration.md), and [Daz Body Authoring](Docs/DazBodyAuthoring.md).

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
