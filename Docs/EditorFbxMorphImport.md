# Editor FBX Morph Import Integration

## Status

BDFR_CharacterBridge now contains an Editor module with a blocking automated FBX character importer.

This is the first stage that moves morph support from metadata-only planning into the real Unreal import path.

## Unreal Engine 5.8 import path

The editor importer uses Unreal's current FBX factory/task APIs:

- `UFbxFactory`
- `UFbxImportUI`
- `UFbxSkeletalMeshImportData`
- `UAssetImportTask`
- `IAssetTools::ImportAssetTasks`

The key skeletal-mesh option is explicitly enabled:

```cpp
ImportUI->SkeletalMeshImportData->bImportMorphTargets = true;
```

The importer is blocking (`bAsync = false`) so the BDFR validation/manifest stage can inspect the completed `USkeletalMesh` immediately after import.

## What happens after Unreal imports the FBX

1. BDFR finds the imported `USkeletalMesh`.
2. It enumerates the actual `UMorphTarget` objects on that mesh.
3. The DTU file is parsed by `FBDFRDazMorphAdapter`.
4. Requested Daz morphs are matched against real imported morph names.
5. Missing DTU-requested morphs are reported.
6. Unlisted imported morphs are retained in the manifest.
7. The manifest is persisted directly on the Skeletal Mesh through `UBDFRCharacterAssetUserData`.
8. The package is marked dirty for saving.

## Persistent asset metadata

`UBDFRCharacterAssetUserData` currently stores:

- source FBX path
- source DTU path
- Daz/BDFR source profile slot
- full `FBDFRMorphTransferPlan`

This means later editor stages do not need to reparse or guess the original Daz morph relationships every time they generate facial/JCM systems.

## Validation modes

By default, missing requested morphs produce warnings because some custom Daz exports intentionally omit certain shapes.

`bStrictMorphValidation` can be enabled to turn missing requested morphs into a failed import result.

## Non-destructive naming

This stage does not destructively rename Unreal `UMorphTarget` objects.

The transfer manifest separately preserves:

- source internal name
- source label
- imported Unreal morph name
- intended target/display name

This avoids breaking curves, JCM relationships or reimport behavior before the full animation/facial pipeline is ready.

## Interchange compatibility direction

Unreal Engine 5.8 also supports skeletal-mesh morph import through Interchange, including its Import Morph Targets option and Interchange mesh utilities.

The BDFR manifest/validation layer is backend-independent by design. A future Interchange backend can feed the same `FBDFRMorphTransferPlan` without changing character runtime metadata.

## Next step

The next editor stage should generate:

1. IK Rig assets from the Daz skeleton profile;
2. IK Retargeter chain mapping;
3. an Unreal Retarget Pose from the BDFR reference-pose conversion plan;
4. JCM Control Rig logic from the persisted `JointDrivenLinks`.
