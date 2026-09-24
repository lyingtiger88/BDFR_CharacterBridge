# Roadmap

## Phase 1 — Foundation
- Unreal Engine 5.8+ plugin module
- normalized character metadata model
- import diagnostics
- source adapter interfaces
- automated tests for mapping data

## Phase 2 — Daz integration
- [x] Genesis 8 skeleton profile foundation
- [x] Genesis 8.1 source-aware profile foundation
- [x] Genesis 9 skeleton profile foundation
- [x] generation detection and validation report
- [x] preserve twist bones as deformation data
- [x] non-destructive reference-pose conversion plan
- [x] component-space pose direction analysis
- [ ] editor preview / IK Retarget Pose generation
- [ ] twist rotation/distribution solver
- [x] morph metadata / DTU manifest parsing
- [x] JCM JointLinks metadata preservation
- [x] editor-side FBX morph-target import integration
- [ ] texture/material metadata handoff

## Phase 3 — Unreal retargeting
- IK Rig generation helpers
- IK Retargeter profile generation
- root-motion handling
- animation sequence transfer
- common-pose validation suite

## Phase 4 — Morph and facial transfer
- [x] Daz morph manifest and name normalization
- [x] semantic morph classification
- [x] joint-driven corrective metadata
- [x] editor-side FBX morph import integration
- [x] persistent morph manifest on Skeletal Mesh asset
- [ ] facial expression semantic mapping
- [ ] ARKit-compatible mapping profile
- [ ] facial animation import
- [ ] validation tools

## Phase 5 — Additional sources
- generic FBX source profile
- Blender-oriented adapter
- MetaHuman interoperability layer
- custom skeleton mapping UI

## Phase 6 — BDFR ecosystem integration
- DynamicBodySystem hooks
- StrandFX hooks
- runtime AI/emotion interfaces
- advanced deformation integration
