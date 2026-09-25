#pragma once

#include "CoreMinimal.h"
#include "Body/BDFRBodyProfileTypes.h"
#include "Morph/BDFRMorphTypes.h"

class USkeletalMesh;
class USkeleton;

struct FBDFRCharacterFbxImportRequest
{
    FString FbxFilename;
    FString DtuFilename;

    /**
     * Optional BDFR body-authoring sidecar. If empty and
     * bAutoDetectBodyProfile is true, CharacterBridge looks for
     * <DTU base name>.bdfrbody.json beside the DTU.
     */
    FString BodyProfileFilename;

    FString DestinationPath = TEXT("/Game/BDFR/Characters");

    USkeleton* ExistingSkeleton = nullptr;

    bool bReplaceExisting = true;
    bool bUseInternalMorphNames = false;
    bool bUseT0AsReferencePose = false;
    bool bForceFrontXAxis = false;
    bool bImportMeshesInBoneHierarchy = true;
    bool bStrictMorphValidation = false;
    bool bAutoDetectBodyProfile = true;
};

struct FBDFRCharacterFbxImportResult
{
    bool bSuccess = false;
    USkeletalMesh* SkeletalMesh = nullptr;

    FBDFRMorphTransferPlan MorphTransferPlan;

    bool bHasBodyProfile = false;
    FBDFRBodyProfile BodyProfile;
    FString ResolvedBodyProfileFilename;

    TArray<FString> Warnings;
    TArray<FString> Errors;
};

/**
 * Blocking editor-side FBX importer used by BDFR character workflows.
 *
 * It explicitly enables morph-target import, builds the Daz morph manifest,
 * auto-loads optional BDFR muscle/soft-tissue authoring metadata, validates
 * imported data, and persists BDFR metadata on the Skeletal Mesh.
 */
struct FBDFRCharacterFbxImporter
{
    static FBDFRCharacterFbxImportResult ImportCharacter(
        const FBDFRCharacterFbxImportRequest& Request);

    static TArray<FName> CollectImportedMorphNames(
        const USkeletalMesh* SkeletalMesh);

private:
    static FString ResolveBodyProfileFilename(
        const FBDFRCharacterFbxImportRequest& Request);

    static void ValidateBodyProfileAnchors(
        const USkeletalMesh* SkeletalMesh,
        const FBDFRBodyProfile& BodyProfile,
        TArray<FString>& OutWarnings);

    static void AttachOrUpdateAssetUserData(
        USkeletalMesh* SkeletalMesh,
        const FBDFRCharacterFbxImportRequest& Request,
        const FBDFRMorphTransferPlan& MorphTransferPlan,
        const FBDFRBodyProfile* BodyProfile,
        const FString& BodyProfileFilename);
};
