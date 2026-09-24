#pragma once

#include "CoreMinimal.h"
#include "Morph/BDFRMorphTypes.h"

class USkeletalMesh;
class USkeleton;

struct FBDFRCharacterFbxImportRequest
{
    FString FbxFilename;
    FString DtuFilename;
    FString DestinationPath = TEXT("/Game/BDFR/Characters");

    USkeleton* ExistingSkeleton = nullptr;

    bool bReplaceExisting = true;
    bool bUseInternalMorphNames = false;
    bool bUseT0AsReferencePose = false;
    bool bForceFrontXAxis = false;
    bool bImportMeshesInBoneHierarchy = true;
    bool bStrictMorphValidation = false;
};

struct FBDFRCharacterFbxImportResult
{
    bool bSuccess = false;
    USkeletalMesh* SkeletalMesh = nullptr;

    FBDFRMorphTransferPlan MorphTransferPlan;

    TArray<FString> Warnings;
    TArray<FString> Errors;
};

/**
 * Blocking editor-side FBX importer used by BDFR character workflows.
 *
 * It explicitly enables morph-target import, builds the Daz morph manifest
 * after Unreal has created the real UMorphTarget objects, validates DTU
 * requests against the imported result, and persists BDFR metadata on the
 * Skeletal Mesh via UBDFRCharacterAssetUserData.
 */
struct FBDFRCharacterFbxImporter
{
    static FBDFRCharacterFbxImportResult ImportCharacter(
        const FBDFRCharacterFbxImportRequest& Request);

    static TArray<FName> CollectImportedMorphNames(
        const USkeletalMesh* SkeletalMesh);

private:
    static void AttachOrUpdateAssetUserData(
        USkeletalMesh* SkeletalMesh,
        const FBDFRCharacterFbxImportRequest& Request,
        const FBDFRMorphTransferPlan& MorphTransferPlan);
};
