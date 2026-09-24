#include "Skeleton/BDFRReferencePoseConverter.h"

#include "ReferenceSkeleton.h"

namespace
{
    FQuat MakeNormalizedDelta(const FQuat& From, const FQuat& To)
    {
        // Delta that rotates From toward To.
        FQuat Delta = To * From.Inverse();
        Delta.Normalize();
        return Delta;
    }

    float AngleBetweenDirectionsDegrees(const FVector& A, const FVector& B)
    {
        if (A.IsNearlyZero() || B.IsNearlyZero())
        {
            return 0.0f;
        }

        const float Dot = FMath::Clamp(
            FVector::DotProduct(A.GetSafeNormal(), B.GetSafeNormal()),
            -1.0f,
            1.0f);

        return FMath::RadiansToDegrees(FMath::Acos(Dot));
    }

    FQuat FindDirectionDelta(const FVector& From, const FVector& To)
    {
        if (From.IsNearlyZero() || To.IsNearlyZero())
        {
            return FQuat::Identity;
        }

        FQuat Delta = FQuat::FindBetweenNormals(
            From.GetSafeNormal(),
            To.GetSafeNormal());

        Delta.Normalize();
        return Delta;
    }
}

FBDFRReferencePoseSnapshot FBDFRReferencePoseConverter::CaptureRawReferencePose(
    const FReferenceSkeleton& ReferenceSkeleton)
{
    FBDFRReferencePoseSnapshot Snapshot;

    const TArray<FMeshBoneInfo>& BoneInfo = ReferenceSkeleton.GetRawRefBoneInfo();
    const TArray<FTransform>& LocalPose = ReferenceSkeleton.GetRawRefBonePose();

    const int32 BoneCount = FMath::Min(BoneInfo.Num(), LocalPose.Num());
    Snapshot.Bones.Reserve(BoneCount);

    for (int32 BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex)
    {
        FBDFRReferenceBonePose Bone;
        Bone.BoneName = BoneInfo[BoneIndex].Name;
        Bone.ParentIndex = ReferenceSkeleton.GetRawParentIndex(BoneIndex);
        Bone.LocalTransform = LocalPose[BoneIndex];

        if (Bone.ParentIndex != INDEX_NONE && Snapshot.Bones.IsValidIndex(Bone.ParentIndex))
        {
            Bone.ComponentTransform =
                Bone.LocalTransform *
                Snapshot.Bones[Bone.ParentIndex].ComponentTransform;
        }
        else
        {
            Bone.ComponentTransform = Bone.LocalTransform;
        }

        Bone.ComponentTransform.NormalizeRotation();
        Snapshot.Bones.Add(Bone);
    }

    return Snapshot;
}

FBDFRReferencePoseConversionPlan FBDFRReferencePoseConverter::BuildPlan(
    const FReferenceSkeleton& SourceReferenceSkeleton,
    const FReferenceSkeleton& TargetReferenceSkeleton,
    const FBDFRSkeletonProfile& Profile)
{
    return BuildPlan(
        CaptureRawReferencePose(SourceReferenceSkeleton),
        CaptureRawReferencePose(TargetReferenceSkeleton),
        Profile);
}

FBDFRReferencePoseConversionPlan FBDFRReferencePoseConverter::BuildPlan(
    const FBDFRReferencePoseSnapshot& SourcePose,
    const FBDFRReferencePoseSnapshot& TargetPose,
    const FBDFRSkeletonProfile& Profile)
{
    FBDFRReferencePoseConversionPlan Plan;
    Plan.SourceProfile = Profile.SourceProfile;

    const int32 SourceRootIndex = SourcePose.FindBoneIndex(Profile.SourceRetargetRoot);
    const int32 TargetRootIndex = TargetPose.FindBoneIndex(Profile.CanonicalRetargetRoot);

    Plan.bHasValidRetargetRoots =
        SourcePose.IsValidIndex(SourceRootIndex) &&
        TargetPose.IsValidIndex(TargetRootIndex);

    if (Plan.bHasValidRetargetRoots)
    {
        Plan.TargetRootTranslationDelta =
            SourcePose.Bones[SourceRootIndex].ComponentTransform.GetTranslation() -
            TargetPose.Bones[TargetRootIndex].ComponentTransform.GetTranslation();
    }

    float DirectionErrorSum = 0.0f;
    int32 DirectionSampleCount = 0;

    for (int32 MappingIndex = 0; MappingIndex < Profile.BoneMappings.Num(); ++MappingIndex)
    {
        const FBDFRBoneMapping& Mapping = Profile.BoneMappings[MappingIndex];

        if (Mapping.RetargetPolicy == EBDFRBoneRetargetPolicy::Excluded)
        {
            continue;
        }

        const int32 SourceBoneIndex = SourcePose.FindBoneIndex(Mapping.SourceBone);
        const int32 TargetBoneIndex = TargetPose.FindBoneIndex(Mapping.CanonicalBone);

        if (!SourcePose.IsValidIndex(SourceBoneIndex))
        {
            if (Mapping.bRequired)
            {
                Plan.MissingSourceBones.AddUnique(Mapping.SourceBone);
            }
            continue;
        }

        if (!TargetPose.IsValidIndex(TargetBoneIndex))
        {
            // Deformation-only source bones are allowed to remain source-specific.
            if (Mapping.RetargetPolicy != EBDFRBoneRetargetPolicy::DeformationOnly)
            {
                Plan.MissingTargetBones.AddUnique(Mapping.CanonicalBone);
            }
            continue;
        }

        FBDFRPoseBoneCorrection Correction;
        Correction.SourceBone = Mapping.SourceBone;
        Correction.TargetBone = Mapping.CanonicalBone;
        Correction.RetargetPolicy = Mapping.RetargetPolicy;

        Correction.LocalReferenceRotationDelta = MakeNormalizedDelta(
            TargetPose.Bones[TargetBoneIndex].LocalTransform.GetRotation(),
            SourcePose.Bones[SourceBoneIndex].LocalTransform.GetRotation());

        const int32 DescendantMappingIndex =
            FindNearestMappedStructuralDescendant(
                SourcePose,
                SourceBoneIndex,
                Profile,
                MappingIndex,
                true);

        if (Profile.BoneMappings.IsValidIndex(DescendantMappingIndex))
        {
            const FBDFRBoneMapping& DescendantMapping =
                Profile.BoneMappings[DescendantMappingIndex];

            const int32 SourceEndIndex =
                SourcePose.FindBoneIndex(DescendantMapping.SourceBone);
            const int32 TargetEndIndex =
                TargetPose.FindBoneIndex(DescendantMapping.CanonicalBone);

            if (SourcePose.IsValidIndex(SourceEndIndex) &&
                TargetPose.IsValidIndex(TargetEndIndex))
            {
                Correction.SourceDirection =
                    SafeDirection(SourcePose, SourceBoneIndex, SourceEndIndex);
                Correction.TargetDirection =
                    SafeDirection(TargetPose, TargetBoneIndex, TargetEndIndex);

                if (!Correction.SourceDirection.IsNearlyZero() &&
                    !Correction.TargetDirection.IsNearlyZero())
                {
                    Correction.bHasDirectionSample = true;
                    Correction.DirectionErrorDegrees =
                        AngleBetweenDirectionsDegrees(
                            Correction.TargetDirection,
                            Correction.SourceDirection);

                    Correction.ComponentAimRotationDelta =
                        FindDirectionDelta(
                            Correction.TargetDirection,
                            Correction.SourceDirection);

                    DirectionErrorSum += Correction.DirectionErrorDegrees;
                    ++DirectionSampleCount;

                    Plan.MaxDirectionErrorDegrees =
                        FMath::Max(
                            Plan.MaxDirectionErrorDegrees,
                            Correction.DirectionErrorDegrees);
                }
            }
        }

        Plan.BoneCorrections.Add(Correction);
    }

    if (DirectionSampleCount > 0)
    {
        Plan.MeanDirectionErrorDegrees =
            DirectionErrorSum / static_cast<float>(DirectionSampleCount);
    }

    return Plan;
}

int32 FBDFRReferencePoseConverter::FindNearestMappedStructuralDescendant(
    const FBDFRReferencePoseSnapshot& Pose,
    const int32 StartBoneIndex,
    const FBDFRSkeletonProfile& Profile,
    const int32 CurrentMappingIndex,
    const bool bUseSourceNames)
{
    if (!Pose.IsValidIndex(StartBoneIndex) ||
        !Profile.BoneMappings.IsValidIndex(CurrentMappingIndex))
    {
        return INDEX_NONE;
    }

    int32 BestMappingIndex = INDEX_NONE;
    int32 BestDistance = MAX_int32;

    for (int32 CandidateMappingIndex = 0;
         CandidateMappingIndex < Profile.BoneMappings.Num();
         ++CandidateMappingIndex)
    {
        if (CandidateMappingIndex == CurrentMappingIndex)
        {
            continue;
        }

        const FBDFRBoneMapping& Candidate =
            Profile.BoneMappings[CandidateMappingIndex];

        if (Candidate.RetargetPolicy == EBDFRBoneRetargetPolicy::Excluded ||
            Candidate.RetargetPolicy == EBDFRBoneRetargetPolicy::DeformationOnly)
        {
            continue;
        }

        const FName CandidateBoneName =
            bUseSourceNames ? Candidate.SourceBone : Candidate.CanonicalBone;

        const int32 CandidateBoneIndex = Pose.FindBoneIndex(CandidateBoneName);
        if (!Pose.IsValidIndex(CandidateBoneIndex))
        {
            continue;
        }

        const int32 Distance =
            GetAncestorDistance(Pose, CandidateBoneIndex, StartBoneIndex);

        if (Distance > 0 && Distance < BestDistance)
        {
            BestDistance = Distance;
            BestMappingIndex = CandidateMappingIndex;
        }
    }

    return BestMappingIndex;
}

int32 FBDFRReferencePoseConverter::GetAncestorDistance(
    const FBDFRReferencePoseSnapshot& Pose,
    int32 DescendantIndex,
    const int32 AncestorIndex)
{
    if (!Pose.IsValidIndex(DescendantIndex) ||
        !Pose.IsValidIndex(AncestorIndex))
    {
        return INDEX_NONE;
    }

    int32 Distance = 0;

    while (Pose.IsValidIndex(DescendantIndex))
    {
        if (DescendantIndex == AncestorIndex)
        {
            return Distance;
        }

        DescendantIndex = Pose.Bones[DescendantIndex].ParentIndex;
        ++Distance;
    }

    return INDEX_NONE;
}

FVector FBDFRReferencePoseConverter::SafeDirection(
    const FBDFRReferencePoseSnapshot& Pose,
    const int32 StartIndex,
    const int32 EndIndex)
{
    if (!Pose.IsValidIndex(StartIndex) || !Pose.IsValidIndex(EndIndex))
    {
        return FVector::ZeroVector;
    }

    return (
        Pose.Bones[EndIndex].ComponentTransform.GetTranslation() -
        Pose.Bones[StartIndex].ComponentTransform.GetTranslation()
    ).GetSafeNormal();
}
