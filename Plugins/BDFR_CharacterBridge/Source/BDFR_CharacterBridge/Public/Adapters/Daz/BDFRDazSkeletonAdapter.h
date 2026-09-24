#pragma once

#include "CoreMinimal.h"
#include "Skeleton/BDFRSkeletonTypes.h"

struct FBDFRDazSkeletonAdapter
{
    static EBDFRCharacterSourceProfile DetectSourceProfile(
        const TArray<FName>& SourceBones,
        const FString& SourceAssetId = FString());

    static FBDFRSkeletonProfile BuildProfile(
        EBDFRCharacterSourceProfile SourceProfile,
        const TArray<FName>& SourceBones = TArray<FName>());

    static FBDFRSkeletonValidationReport Validate(
        const FBDFRSkeletonProfile& Profile,
        const TArray<FName>& SourceBones);

    static bool IsTwistBone(FName BoneName);

private:
    static void AddDetectedTwistBones(
        FBDFRSkeletonProfile& Profile,
        const TArray<FName>& SourceBones);
};
