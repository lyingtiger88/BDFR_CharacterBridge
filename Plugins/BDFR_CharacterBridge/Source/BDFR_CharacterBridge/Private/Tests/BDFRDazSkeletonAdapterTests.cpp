#if WITH_DEV_AUTOMATION_TESTS

#include "Adapters/Daz/BDFRDazSkeletonAdapter.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRDazGenesis9DetectionTest,
    "BDFR.CharacterBridge.Daz.Skeleton.DetectGenesis9",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBDFRDazGenesis9DetectionTest::RunTest(const FString& Parameters)
{
    const TArray<FName> Bones =
    {
        TEXT("hip"), TEXT("pelvis"), TEXT("spine1"), TEXT("spine4"),
        TEXT("l_upperarm"), TEXT("r_upperarm"), TEXT("l_thigh"), TEXT("r_thigh")
    };

    TestEqual(
        TEXT("Genesis 9 signature should be detected"),
        FBDFRDazSkeletonAdapter::DetectSourceProfile(Bones),
        EBDFRCharacterSourceProfile::DazGenesis9);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRDazGenesis81HintDetectionTest,
    "BDFR.CharacterBridge.Daz.Skeleton.DetectGenesis81WithSourceHint",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBDFRDazGenesis81HintDetectionTest::RunTest(const FString& Parameters)
{
    const TArray<FName> Bones =
    {
        TEXT("hip"), TEXT("abdomenLower"), TEXT("abdomenUpper"), TEXT("chestUpper"),
        TEXT("lShldrBend"), TEXT("rShldrBend"), TEXT("lThighBend"), TEXT("rThighBend")
    };

    TestEqual(
        TEXT("Genesis 8.1 should be selected when the shared G8 skeleton is paired with a G8.1 source hint"),
        FBDFRDazSkeletonAdapter::DetectSourceProfile(Bones, TEXT("Genesis8_1Female")),
        EBDFRCharacterSourceProfile::DazGenesis81);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRDazTwistPreservationTest,
    "BDFR.CharacterBridge.Daz.Skeleton.PreserveTwistBones",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBDFRDazTwistPreservationTest::RunTest(const FString& Parameters)
{
    const TArray<FName> Bones =
    {
        TEXT("hip"), TEXT("abdomenLower"), TEXT("abdomenUpper"), TEXT("chestUpper"),
        TEXT("lShldrBend"), TEXT("rShldrBend"), TEXT("lThighBend"), TEXT("rThighBend"),
        TEXT("lShldrTwist"), TEXT("customArmTwist02")
    };

    const FBDFRSkeletonProfile Profile =
        FBDFRDazSkeletonAdapter::BuildProfile(EBDFRCharacterSourceProfile::DazGenesis8, Bones);

    const FBDFRBoneMapping* CustomTwist = Profile.BoneMappings.FindByPredicate(
        [](const FBDFRBoneMapping& Mapping)
        {
            return Mapping.SourceBone == FName(TEXT("customArmTwist02"));
        });

    TestNotNull(TEXT("Unknown twist bones should be preserved instead of removed"), CustomTwist);

    if (CustomTwist)
    {
        TestEqual(
            TEXT("Detected twist bones should be deformation-only for retargeting"),
            CustomTwist->RetargetPolicy,
            EBDFRBoneRetargetPolicy::DeformationOnly);
    }

    return true;
}

#endif
