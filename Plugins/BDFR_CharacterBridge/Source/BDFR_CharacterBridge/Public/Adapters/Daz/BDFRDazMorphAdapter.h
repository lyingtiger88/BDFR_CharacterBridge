#pragma once

#include "CoreMinimal.h"
#include "Morph/BDFRMorphTypes.h"

struct FBDFRDazMorphAdapter
{
    static bool BuildTransferPlanFromDtuJson(
        const FString& DtuJson,
        const TArray<FName>& ImportedMorphNames,
        FBDFRMorphTransferPlan& OutPlan,
        bool bUseInternalMorphNames = false);

    static FName NormalizeDazMorphName(const FString& MorphName);

    static FName NormalizeFbxMorphChannelName(const FString& ChannelName);

    static EBDFRMorphSemantic ClassifyMorph(const FString& MorphName, const FString& MorphLabel);

    static bool IsLikelyJointCorrectiveMorph(const FString& MorphName);
};
