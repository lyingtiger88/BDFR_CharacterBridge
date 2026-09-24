#if WITH_DEV_AUTOMATION_TESTS

#include "Skeleton/BDFRReferencePoseConverter.h"
#include "Misc/AutomationTest.h"

namespace
{
    FBDFRReferenceBonePose MakeBone(
        const TCHAR* Name,
        const int32 ParentIndex,
        const FVector& ComponentLocation,
        const FQuat& LocalRotation = FQuat::Identity)
    {
        FBDFRReferenceBonePose Bone;
        Bone.BoneName = FName(Name);
        Bone.ParentIndex = ParentIndex;
        Bone.LocalTransform = FTransform(LocalRotation, FVector::ZeroVector);
        Bone.ComponentTransform = FTransform(
            LocalRotation,
            ComponentLocation,
            FVector::OneVector);
        return Bone;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRReferencePoseDirectionPlanTest,
    "BDFR.CharacterBridge.Skeleton.ReferencePose.DirectionAlignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBDFRReferencePoseDirectionPlanTest::RunTest(const FString& Parameters)
{
    FBDFRReferencePoseSnapshot Source;
    Source.Bones =
    {
        MakeBone(TEXT("hip"), INDEX_NONE, FVector::ZeroVector),
        MakeBone(TEXT("lShldrBend"), 0, FVector::ZeroVector),
        MakeBone(TEXT("lForearmBend"), 1, FVector(100.0, 0.0, 0.0))
    };

    FBDFRReferencePoseSnapshot Target;
    Target.Bones =
    {
        MakeBone(TEXT("pelvis"), INDEX_NONE, FVector::ZeroVector),
        MakeBone(TEXT("upperarm_l"), 0, FVector::ZeroVector),
        MakeBone(TEXT("lowerarm_l"), 1, FVector(0.0, 100.0, 0.0))
    };

    FBDFRSkeletonProfile Profile;
    Profile.SourceProfile = EBDFRCharacterSourceProfile::DazGenesis8;
    Profile.SourceRetargetRoot = TEXT("hip");
    Profile.CanonicalRetargetRoot = TEXT("pelvis");

    FBDFRBoneMapping Shoulder;
    Shoulder.SourceBone = TEXT("lShldrBend");
    Shoulder.CanonicalBone = TEXT("upperarm_l");
    Shoulder.bRequired = true;

    FBDFRBoneMapping Forearm;
    Forearm.SourceBone = TEXT("lForearmBend");
    Forearm.CanonicalBone = TEXT("lowerarm_l");
    Forearm.bRequired = true;

    Profile.BoneMappings = { Shoulder, Forearm };

    const FBDFRReferencePoseConversionPlan Plan =
        FBDFRReferencePoseConverter::BuildPlan(Source, Target, Profile);

    TestTrue(TEXT("Plan should have valid retarget roots"), Plan.bHasValidRetargetRoots);
    TestEqual(TEXT("Two mapped bones should generate corrections"), Plan.BoneCorrections.Num(), 2);

    const FBDFRPoseBoneCorrection* UpperArmCorrection =
        Plan.BoneCorrections.FindByPredicate(
            [](const FBDFRPoseBoneCorrection& Correction)
            {
                return Correction.SourceBone == FName(TEXT("lShldrBend"));
            });

    TestNotNull(TEXT("Upper-arm correction should exist"), UpperArmCorrection);

    if (UpperArmCorrection)
    {
        TestTrue(TEXT("Upper arm should have a direction sample"), UpperArmCorrection->bHasDirectionSample);
        TestTrue(
            TEXT("Direction error should be approximately 90 degrees"),
            FMath::IsNearlyEqual(
                UpperArmCorrection->DirectionErrorDegrees,
                90.0f,
                0.1f));

        const FVector RotatedTarget =
            UpperArmCorrection->ComponentAimRotationDelta.RotateVector(
                UpperArmCorrection->TargetDirection);

        TestTrue(
            TEXT("Aim delta should rotate the target direction toward the source direction"),
            RotatedTarget.Equals(
                UpperArmCorrection->SourceDirection,
                0.001f));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRReferencePoseRootTranslationTest,
    "BDFR.CharacterBridge.Skeleton.ReferencePose.RootTranslation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBDFRReferencePoseRootTranslationTest::RunTest(const FString& Parameters)
{
    FBDFRReferencePoseSnapshot Source;
    Source.Bones =
    {
        MakeBone(TEXT("hip"), INDEX_NONE, FVector(0.0, 0.0, 100.0))
    };

    FBDFRReferencePoseSnapshot Target;
    Target.Bones =
    {
        MakeBone(TEXT("pelvis"), INDEX_NONE, FVector(0.0, 0.0, 90.0))
    };

    FBDFRSkeletonProfile Profile;
    Profile.SourceProfile = EBDFRCharacterSourceProfile::DazGenesis8;
    Profile.SourceRetargetRoot = TEXT("hip");
    Profile.CanonicalRetargetRoot = TEXT("pelvis");

    FBDFRBoneMapping Root;
    Root.SourceBone = TEXT("hip");
    Root.CanonicalBone = TEXT("pelvis");
    Root.bRequired = true;
    Profile.BoneMappings.Add(Root);

    const FBDFRReferencePoseConversionPlan Plan =
        FBDFRReferencePoseConverter::BuildPlan(Source, Target, Profile);

    TestTrue(
        TEXT("Root translation delta should preserve the measured component-space offset"),
        Plan.TargetRootTranslationDelta.Equals(
            FVector(0.0, 0.0, 10.0),
            KINDA_SMALL_NUMBER));

    return true;
}

#endif
