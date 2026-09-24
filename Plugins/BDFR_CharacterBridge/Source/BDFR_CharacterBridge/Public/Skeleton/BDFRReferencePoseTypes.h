#pragma once

#include "CoreMinimal.h"
#include "Skeleton/BDFRSkeletonTypes.h"
#include "BDFRReferencePoseTypes.generated.h"

USTRUCT(BlueprintType)
struct FBDFRReferenceBonePose
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FName BoneName = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    int32 ParentIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FTransform LocalTransform = FTransform::Identity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FTransform ComponentTransform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct FBDFRReferencePoseSnapshot
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    TArray<FBDFRReferenceBonePose> Bones;

    int32 FindBoneIndex(const FName BoneName) const
    {
        return Bones.IndexOfByPredicate(
            [BoneName](const FBDFRReferenceBonePose& Bone)
            {
                return Bone.BoneName == BoneName;
            });
    }

    bool IsValidIndex(const int32 BoneIndex) const
    {
        return Bones.IsValidIndex(BoneIndex);
    }
};

USTRUCT(BlueprintType)
struct FBDFRPoseBoneCorrection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FName SourceBone = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FName TargetBone = NAME_None;

    /**
     * Candidate local-space rotation delta from the target reference orientation
     * toward the mapped source orientation. This is diagnostic/conversion-plan
     * data and is not applied destructively to the target skeleton.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FQuat LocalReferenceRotationDelta = FQuat::Identity;

    /**
     * Component-space rotation that aligns the target bone-to-child direction
     * with the corresponding source direction.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FQuat ComponentAimRotationDelta = FQuat::Identity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FVector SourceDirection = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FVector TargetDirection = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    float DirectionErrorDegrees = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    bool bHasDirectionSample = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    EBDFRBoneRetargetPolicy RetargetPolicy = EBDFRBoneRetargetPolicy::Full;
};

USTRUCT(BlueprintType)
struct FBDFRReferencePoseConversionPlan
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    EBDFRCharacterSourceProfile SourceProfile = EBDFRCharacterSourceProfile::Unknown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    TArray<FBDFRPoseBoneCorrection> BoneCorrections;

    /**
     * Component-space translation that would move the target retarget root
     * to the source retarget-root position before any proportion-aware solve.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    FVector TargetRootTranslationDelta = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    TArray<FName> MissingSourceBones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    TArray<FName> MissingTargetBones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    float MeanDirectionErrorDegrees = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    float MaxDirectionErrorDegrees = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|ReferencePose")
    bool bHasValidRetargetRoots = false;

    bool IsUsable() const
    {
        return bHasValidRetargetRoots && MissingSourceBones.Num() == 0;
    }
};
