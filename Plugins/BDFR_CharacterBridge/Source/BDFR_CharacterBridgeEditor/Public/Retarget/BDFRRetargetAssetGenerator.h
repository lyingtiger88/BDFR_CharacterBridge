#pragma once

#include "CoreMinimal.h"
#include "Skeleton/BDFRSkeletonTypes.h"

class UIKRigDefinition;
class UIKRetargeter;
class USkeletalMesh;

struct FBDFRResolvedRetargetChain
{
    FName ChainName = NAME_None;
    FName SourceStartBone = NAME_None;
    FName SourceEndBone = NAME_None;
    FName TargetStartBone = NAME_None;
    FName TargetEndBone = NAME_None;
};

struct FBDFRRetargetAssetGenerationRequest
{
    USkeletalMesh* SourceMesh = nullptr;
    USkeletalMesh* TargetMesh = nullptr;

    EBDFRCharacterSourceProfile SourceProfile =
        EBDFRCharacterSourceProfile::Unknown;

    FString SourceAssetHint;
    FString DestinationPath = TEXT("/Game/BDFR/Retarget");

    FString SourceIKRigName;
    FString TargetIKRigName;
    FString RetargeterName;

    bool bOverwriteExisting = true;
    bool bTryAutoFBIK = true;

    /**
     * UE 5.8 ChainToChain Auto Align is the default because it is robust to
     * different source/target local bone-axis conventions.
     */
    bool bUseUnrealAutoAlign = true;

    /**
     * If Auto Align is disabled, apply the non-destructive BDFR reference-pose
     * conversion plan directly as target local rotation/root offsets.
     */
    bool bApplyBDFRReferencePosePlan = true;
};

struct FBDFRRetargetAssetGenerationResult
{
    bool bSuccess = false;

    UIKRigDefinition* SourceIKRig = nullptr;
    UIKRigDefinition* TargetIKRig = nullptr;
    UIKRetargeter* Retargeter = nullptr;

    FName TargetRetargetPoseName = NAME_None;

    TArray<FBDFRResolvedRetargetChain> ResolvedChains;
    TArray<FString> Warnings;
    TArray<FString> Errors;
};

/**
 * Generates UE 5.8 IK Rig and IK Retargeter assets from a BDFR/Daz skeleton
 * profile. The source Daz hierarchy is kept intact.
 */
struct FBDFRRetargetAssetGenerator
{
    static FBDFRRetargetAssetGenerationResult Generate(
        const FBDFRRetargetAssetGenerationRequest& Request);

    /**
     * Pure planning helper used by tests and diagnostics.
     */
    static TArray<FBDFRResolvedRetargetChain> ResolveCompatibleChains(
        const FBDFRSkeletonProfile& Profile,
        const TArray<FName>& SourceBones,
        const TArray<FName>& TargetBones,
        TArray<FString>& OutWarnings);

private:
    static TArray<FName> CollectRawBoneNames(const USkeletalMesh* Mesh);
};
