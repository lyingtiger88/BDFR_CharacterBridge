#pragma once

#include "CoreMinimal.h"
#include "Skeleton/BDFRReferencePoseTypes.h"

struct FReferenceSkeleton;

/**
 * Builds a non-destructive conversion plan between a source character reference
 * pose (for example Daz Genesis) and an Unreal/BDFR target reference pose.
 *
 * The converter deliberately does not edit USkeleton or USkeletalMesh assets.
 * Editor tooling can later consume the plan to create IK Retarget poses,
 * converted assets, or validation previews.
 */
struct FBDFRReferencePoseConverter
{
    static FBDFRReferencePoseSnapshot CaptureRawReferencePose(
        const FReferenceSkeleton& ReferenceSkeleton);

    static FBDFRReferencePoseConversionPlan BuildPlan(
        const FReferenceSkeleton& SourceReferenceSkeleton,
        const FReferenceSkeleton& TargetReferenceSkeleton,
        const FBDFRSkeletonProfile& Profile);

    static FBDFRReferencePoseConversionPlan BuildPlan(
        const FBDFRReferencePoseSnapshot& SourcePose,
        const FBDFRReferencePoseSnapshot& TargetPose,
        const FBDFRSkeletonProfile& Profile);

private:
    static int32 FindNearestMappedStructuralDescendant(
        const FBDFRReferencePoseSnapshot& Pose,
        int32 StartBoneIndex,
        const FBDFRSkeletonProfile& Profile,
        int32 CurrentMappingIndex,
        bool bUseSourceNames);

    static int32 GetAncestorDistance(
        const FBDFRReferencePoseSnapshot& Pose,
        int32 DescendantIndex,
        int32 AncestorIndex);

    static FVector SafeDirection(
        const FBDFRReferencePoseSnapshot& Pose,
        int32 StartIndex,
        int32 EndIndex);
};
