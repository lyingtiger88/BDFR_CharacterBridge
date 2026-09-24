# Daz to Unreal Skeleton Conversion

## Goal

Convert Daz Genesis skeletal data into an Unreal-friendly, retargetable representation while preserving deformation quality and source-specific information that cannot be represented by simple bone renaming.

## Conversion stages

### 1. Source profile detection
Identify the supported Genesis generation and load its semantic mapping profile.

Initial targets:
- Genesis 8
- Genesis 8.1
- Genesis 9

### 2. Skeleton capture
Record:
- hierarchy
- local and component-space transforms
- reference pose
- bone lengths
- source names
- side/semantic information
- candidate twist and corrective bones

### 3. Semantic mapping
Map source bones to BDFR semantic roles such as pelvis, spine, clavicle, upper arm, lower arm, hand, thigh, calf, foot and facial/jaw roles.

Semantic identity is kept separate from target Unreal bone names.

### 4. Reference-pose conversion
Correct differences in source reference pose before retargeting. Pose correction must not silently rewrite skin deformation assumptions.

### 5. Twist handling
Daz twist/distribution bones require explicit handling. Depending on the target profile, the bridge may:
- retain a source twist bone
- map it to a target twist chain
- redistribute rotation
- mark it as deformation-only

The converter should not simply delete twist bones when doing so would degrade skinning.

### 6. Coordinate normalization
Normalize source scale, handedness and axis conventions into Unreal space.

### 7. Retarget profile
Generate or populate data needed for Unreal IK Rig / IK Retargeter workflows.

### 8. Validation
Run structural and pose tests for:
- shoulders
- elbows
- forearms
- wrists
- spine
- hips
- knees
- ankles
- feet

The validation report should expose unresolved mappings and suspicious transform differences.

## Non-goals

The first skeleton-conversion implementation is not intended to physically simulate muscles or soft tissue. Those systems belong to downstream deformation/runtime modules and should consume the normalized character output.


## Implemented reference-pose planning

The runtime foundation now includes `FBDFRReferencePoseConverter`, which captures Unreal raw reference poses and builds a non-destructive conversion plan between the Daz source profile and a canonical target skeleton.

The plan records local reference-rotation deltas, component-space limb direction differences, root translation delta, missing mappings, and angular error metrics.

See [Reference Pose Conversion](ReferencePoseConversion.md).
