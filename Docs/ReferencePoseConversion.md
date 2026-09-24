# Reference Pose Conversion

## Purpose

Daz and Unreal characters can have different reference poses, bone orientations and proportions even when their semantic body structure is equivalent.

BDFR_CharacterBridge therefore treats reference-pose conversion as a **non-destructive planning stage**.

The converter does not rewrite a `USkeleton` or `USkeletalMesh`. Instead it captures both reference skeletons and produces a conversion plan that can later be consumed by Editor tooling or an IK Retargeter generator.

## Unreal Engine 5.8 model

Unreal Engine 5.8's IK Retargeter supports custom Retarget Poses specifically to compensate for source/target reference-pose differences such as A-pose versus T-pose.

`FIKRetargetPose` stores local-space bone rotation deltas and a global root-translation delta. BDFR's plan is intentionally structured around the same concepts so a later editor module can generate Unreal-native retarget data without destructively changing the imported source skeleton.

## Captured data

For every raw reference bone the snapshot stores:

- bone name
- parent index
- local reference transform
- component-space reference transform

The capture layer uses Unreal's `FReferenceSkeleton` raw reference data so virtual bones do not accidentally alter source-profile detection or the original imported deformation hierarchy.

## Conversion plan

For every mapped structural bone the plan can contain:

- source bone
- target/canonical bone
- candidate local reference-rotation delta
- source bone-to-descendant direction
- target bone-to-descendant direction
- component-space aim delta
- angular direction error
- retarget policy

At the plan level it also records:

- target root translation delta
- missing required source bones
- missing target canonical bones
- mean direction error
- maximum direction error
- retarget-root validity

## Why both local and directional deltas are stored

A direct local-rotation comparison is useful diagnostic information, but local bone bases may differ across skeleton conventions.

The directional measurement instead asks a more robust geometric question:

> In component space, where does this limb segment point in the source pose versus the target pose?

The first implementation therefore records both. The later solver can choose how to blend or convert these measurements into Unreal IK Retarget Pose local offsets.

## Twist bones

Twist bones marked `DeformationOnly` are intentionally not used as structural direction targets. This prevents an intermediate twist helper from becoming the limb endpoint used for A/T-pose alignment.

Their source deformation data remains preserved for skinning and future twist-distribution work.

## Current safety rule

This stage **never applies corrections directly**.

That means a wrong mapping or unusual custom Daz rig produces a bad plan/validation report, not a damaged Skeleton asset.

## Next step

The next editor-side layer should:

1. load a source `USkeletalMesh`;
2. select or generate a canonical target profile;
3. run `FBDFRReferencePoseConverter`;
4. preview angular corrections;
5. convert the approved plan into an Unreal 5.8 IK Retarget Pose;
6. create the IK Rig and Retargeter assets;
7. keep the original source skeleton intact.
