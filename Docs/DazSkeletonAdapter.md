# Daz Skeleton Adapter

## Status

The first Daz skeleton-adapter layer is now implemented as a data-driven foundation for Genesis 8, Genesis 8.1 and Genesis 9.

This stage intentionally does **not** rewrite imported Unreal skeleton assets yet. It establishes detection, canonical mapping, retarget-chain definitions, twist preservation and validation so the later importer/editor stage can make controlled changes.

## What was learned from DazToUnreal

The current DazToUnreal implementation identifies common character generations from skeletal signatures when creating IK rigs.

Examples used by its current scripts include:

- Genesis 9: `hip`, `pelvis`, `spine1`, `spine4`
- Genesis 8 family: `hip`, `abdomenLower`, `abdomenUpper`, `chestUpper`

Its IK setup also defines generation-specific arm, leg, spine, head and finger chains.

The existing DazToUnreal FBX twist fix takes bones whose names contain `twist` "out of line" by reparenting their children. BDFR does not use that as its default strategy. Twist bones are preserved and marked as deformation-only unless a profile explicitly decides otherwise.

Reference implementation studied:

- DazToUnreal `Content/Python/CreateIKRig.py`
- DazToUnreal `Content/Python/CreateIKRetargeter.py`
- DazToUnreal `Private/DazToUnrealFbx.cpp`

## Detection

### Genesis 9

The adapter detects Genesis 9 from a stronger body signature including:

```text
hip
pelvis
spine1
spine4
l_upperarm
r_upperarm
l_thigh
r_thigh
```

### Genesis 8 / 8.1

Genesis 8 and Genesis 8.1 share the body-skeleton convention closely enough that skeleton names alone are not treated as a reliable 8-vs-8.1 discriminator.

The adapter therefore:

1. detects the Genesis 8 family from body bones;
2. uses the Daz source asset identifier when available to distinguish Genesis 8.1;
3. falls back to Genesis 8 when no 8.1 source hint exists.

This avoids inventing a false structural distinction.

### Genesis 3 guard

DazToUnreal's own detection logic shows that Genesis 3 can match the Genesis 8-style signature and uses `lHeel` as an additional discriminator. BDFR currently treats that case as unsupported instead of incorrectly importing it as Genesis 8.

## Canonical skeleton

The current canonical body naming is Unreal-friendly:

```text
pelvis
spine_01
spine_02
spine_03
spine_04
neck_01
neck_02
head

clavicle_l / clavicle_r
upperarm_l / upperarm_r
lowerarm_l / lowerarm_r
hand_l / hand_r

thigh_l / thigh_r
calf_l / calf_r
foot_l / foot_r
ball_l / ball_r
```

Finger chains use the familiar `thumb_01_l`, `index_01_l`, etc. convention.

The canonical schema is intentionally metadata at this stage. Actual destructive renaming/reparenting will be performed later by an editor conversion layer.

## Twist policy

Twist handling follows four rules:

1. never remove a twist bone simply because its name contains `twist`;
2. preserve known Genesis 8 arm/forearm/thigh twist bones;
3. preserve unknown twist-named bones discovered in the source skeleton;
4. mark twist bones as `DeformationOnly` for retargeting by default.

This allows skin weights and future corrective deformation systems to retain access to the source deformation chain.

## Validation

The adapter reports:

- missing required bones
- whether the source retarget root exists
- unmapped source bones
- detected twist bones

Unmapped bones are not automatically errors because Daz characters may contain facial, corrective, helper or attachment bones that are outside the initial body-retarget profile.

## Next implementation step

The next layer should consume `FBDFRSkeletonProfile` in an Unreal Editor module and:

1. inspect an imported `USkeletalMesh` / `USkeleton`;
2. build a conversion preview;
3. create IK Rig chain data;
4. calculate reference-pose corrections;
5. create a non-destructive converted skeleton/retarget profile;
6. run the validation report before saving generated assets.
