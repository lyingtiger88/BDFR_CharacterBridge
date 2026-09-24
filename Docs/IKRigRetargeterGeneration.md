# IK Rig and IK Retargeter Generation

## Status

BDFR_CharacterBridge now contains an editor-side retarget asset generator targeting Unreal Engine 5.8.

The generator creates:

- source IK Rig for the imported Daz character
- target IK Rig for an Unreal/canonical character
- matching retarget chains
- IK Retargeter asset
- default UE 5.8 retarget operation stack
- exact chain mappings
- a dedicated target Retarget Pose
- persistent references back on the imported BDFR Skeletal Mesh

## Source profile detection

If no source profile is explicitly supplied, the generator reads the source Skeletal Mesh raw reference bone names and uses the existing Daz adapter to detect:

- Genesis 8
- Genesis 8.1 when a suitable source hint exists
- Genesis 9

Unsupported/ambiguous skeletons fail early rather than silently receiving an incorrect chain layout.

## IK Rig generation

Each IK Rig is assigned its real Skeletal Mesh through the UE 5.8 `UIKRigController`.

The Daz side uses source chain names from `FBDFRSkeletonProfile`.

The target side uses the corresponding BDFR canonical Unreal bone names.

Both assets receive the same semantic chain names, such as:

```text
Spine
Head
LeftClavicle
RightClavicle
LeftArm
RightArm
LeftLeg
RightLeg
LeftThumb
RightThumb
...
```

Using identical semantic chain names makes exact source/target mapping deterministic.

## UE5 Manny / Quinn torso handling

The canonical BDFR profile currently maps the Daz torso through `spine_04`.

When a target skeleton also contains `spine_05`, the generated target `Spine` chain is extended to `spine_05` so UE5 Manny/Quinn-style torso hierarchies are not artificially truncated.

## FBIK

The generator can ask UE 5.8 to run its built-in Auto FBIK setup.

This is expected to work well for Unreal-recognized target skeletons such as Manny/Quinn and MetaHuman-like biped templates.

Daz source skeletons may not match Epic's built-in FBIK templates. Failure is therefore reported as a warning, not a fatal error; the manually generated retarget chains remain valid.

A dedicated BDFR Daz FBIK generator can later add Daz-specific goals and preferred angles.

## IK Retargeter generation

The generator assigns source and target IK Rigs, preview meshes, and then requests the default UE 5.8 retarget operation stack.

It propagates the assigned IK Rigs to all compatible retarget operations and uses exact chain mapping.

## Retarget Pose

A target pose named `BDFR_Aligned` is created automatically.

### Default: UE 5.8 ChainToChain Auto Align

The default behavior uses Unreal Engine 5.8's `AutoAlignAllBones` with `ChainToChain`.

This is preferred because Daz and Unreal can use different local bone-axis conventions, and UE's retarget pose generator is designed to reason about chain direction rather than assuming that local rotation axes are equivalent.

### BDFR manual fallback

Auto Align can be disabled.

In that mode, the generator can consume the existing `FBDFRReferencePoseConversionPlan` and write:

- per-bone local rotation offsets
- retarget root translation offset

directly into the target Retarget Pose.

This path is useful for diagnostics and future custom solvers, but Auto Align is the recommended default.

## Persistent asset metadata

The imported source Skeletal Mesh stores soft references to:

- generated source IK Rig
- generated target IK Rig
- generated IK Retargeter
- generated target Retarget Pose name

inside `UBDFRCharacterAssetUserData`.

This allows later animation-transfer tools to discover the correct retargeter without asking the user to manually reconnect assets.

## Next step

With the retarget infrastructure in place, the next stage is animation transfer:

1. import Daz Animation Sequences against the source skeleton;
2. preserve root-motion metadata;
3. batch-retarget selected animations through the generated IK Retargeter;
4. validate locomotion/pose output;
5. add runtime retarget support for Live Link or dynamic character swapping.
