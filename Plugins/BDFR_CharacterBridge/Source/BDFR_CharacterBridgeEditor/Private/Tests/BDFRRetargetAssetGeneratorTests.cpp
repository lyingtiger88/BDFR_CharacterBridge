#if WITH_DEV_AUTOMATION_TESTS

#include "Adapters/Daz/BDFRDazSkeletonAdapter.h"
#include "Misc/AutomationTest.h"
#include "Retarget/BDFRRetargetAssetGenerator.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRRetargetChainResolutionTest,
    "BDFR.CharacterBridge.Retarget.ResolveCompatibleChains",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FBDFRRetargetChainResolutionTest::RunTest(
    const FString& Parameters)
{
    const FBDFRSkeletonProfile Profile =
        FBDFRDazSkeletonAdapter::BuildProfile(
            EBDFRCharacterSourceProfile::DazGenesis8);

    const TArray<FName> SourceBones =
    {
        TEXT("hip"),
        TEXT("abdomenLower"),
        TEXT("abdomenUpper"),
        TEXT("chestLower"),
        TEXT("chestUpper"),
        TEXT("neckLower"),
        TEXT("head"),
        TEXT("lCollar"),
        TEXT("lShldrBend"),
        TEXT("lHand"),
        TEXT("rCollar"),
        TEXT("rShldrBend"),
        TEXT("rHand"),
        TEXT("lThighBend"),
        TEXT("lToe"),
        TEXT("rThighBend"),
        TEXT("rToe")
    };

    const TArray<FName> TargetBones =
    {
        TEXT("pelvis"),
        TEXT("spine_01"),
        TEXT("spine_02"),
        TEXT("spine_03"),
        TEXT("spine_04"),
        TEXT("spine_05"),
        TEXT("neck_01"),
        TEXT("head"),
        TEXT("clavicle_l"),
        TEXT("upperarm_l"),
        TEXT("hand_l"),
        TEXT("clavicle_r"),
        TEXT("upperarm_r"),
        TEXT("hand_r"),
        TEXT("thigh_l"),
        TEXT("ball_l"),
        TEXT("thigh_r"),
        TEXT("ball_r")
    };

    TArray<FString> Warnings;
    const TArray<FBDFRResolvedRetargetChain> Chains =
        FBDFRRetargetAssetGenerator::ResolveCompatibleChains(
            Profile,
            SourceBones,
            TargetBones,
            Warnings);

    const FBDFRResolvedRetargetChain* Spine =
        Chains.FindByPredicate(
            [](const FBDFRResolvedRetargetChain& Chain)
            {
                return Chain.ChainName ==
                    FName(TEXT("Spine"));
            });

    TestNotNull(
        TEXT("Spine chain should resolve"),
        Spine);

    if (Spine)
    {
        TestEqual(
            TEXT("UE5 extended torso should include spine_05 when present"),
            Spine->TargetEndBone,
            FName(TEXT("spine_05")));
    }

    const FBDFRResolvedRetargetChain* LeftArm =
        Chains.FindByPredicate(
            [](const FBDFRResolvedRetargetChain& Chain)
            {
                return Chain.ChainName ==
                    FName(TEXT("LeftArm"));
            });

    TestNotNull(
        TEXT("Left arm chain should resolve"),
        LeftArm);

    return true;
}

#endif
