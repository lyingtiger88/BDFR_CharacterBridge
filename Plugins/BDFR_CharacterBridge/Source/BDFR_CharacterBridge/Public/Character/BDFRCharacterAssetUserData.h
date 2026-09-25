#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "Body/BDFRBodyProfileTypes.h"
#include "Morph/BDFRMorphTypes.h"
#include "Skeleton/BDFRSkeletonTypes.h"
#include "BDFRCharacterAssetUserData.generated.h"

/**
 * Persistent BDFR metadata attached directly to an imported Skeletal Mesh.
 *
 * The geometry/morph deltas remain owned by Unreal's Skeletal Mesh and
 * UMorphTarget objects. This object preserves Daz/BDFR semantic metadata
 * needed by later retarget, facial and JCM generation stages.
 */
UCLASS(BlueprintType, EditInlineNew)
class BDFR_CHARACTERBRIDGE_API UBDFRCharacterAssetUserData : public UAssetUserData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Source")
    FString SourceFbxFile;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Source")
    FString SourceDtuFile;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Source")
    EBDFRCharacterSourceProfile SourceProfile = EBDFRCharacterSourceProfile::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    FBDFRMorphTransferPlan MorphTransferPlan;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FString SourceBodyProfileFile;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    bool bHasBodyProfile = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FBDFRBodyProfile BodyProfile;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Retarget")
    FSoftObjectPath GeneratedSourceIKRig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Retarget")
    FSoftObjectPath GeneratedTargetIKRig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Retarget")
    FSoftObjectPath GeneratedIKRetargeter;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Retarget")
    FName GeneratedTargetRetargetPose = NAME_None;
};
