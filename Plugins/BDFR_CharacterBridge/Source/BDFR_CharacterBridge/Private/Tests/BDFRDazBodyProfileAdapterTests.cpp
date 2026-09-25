#if WITH_DEV_AUTOMATION_TESTS

#include "Adapters/Daz/BDFRDazBodyProfileAdapter.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRDazBodyProfileParseTest,
    "BDFR.CharacterBridge.Daz.BodyProfile.Parse",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FBDFRDazBodyProfileParseTest::RunTest(
    const FString& Parameters)
{
    const FString Json = TEXT(R"JSON(
    {
        "schema": "BDFR.BodyProfile",
        "version": 1,
        "units": "cm",
        "coordinateSystem": "DazStudio",
        "characterName": "Genesis9",
        "animationClips": [
            {
                "name": "Walk",
                "type": "Walk",
                "sourceFile": "C:/BDFR/Walk.duf",
                "loop": true
            },
            {
                "name": "Run",
                "type": "Run",
                "sourceFile": "C:/BDFR/Run.duf",
                "loop": true
            }
        ],
        "regions": [
            {
                "id": "BDFR_Muscle_Biceps_L",
                "name": "Biceps",
                "type": "Muscle",
                "side": "Left",
                "anchorA": "l_upperarm",
                "anchorB": "l_forearm",
                "localPosition": [1.0, 2.0, 3.0],
                "localRotation": [0.0, 0.0, 0.0, 1.0],
                "worldPosition": [4.0, 5.0, 6.0],
                "worldRotation": [0.0, 0.0, 0.0, 1.0],
                "radiusCm": [3.0, 2.0, 8.0],
                "massKg": 0.4,
                "stiffness": 0.8,
                "damping": 0.3,
                "compliance": 0.2,
                "activationScale": 1.0,
                "maxBulge": 0.15,
                "collisionScale": 1.0,
                "enabled": true
            }
        ]
    }
    )JSON");

    FBDFRBodyProfile Profile;
    TArray<FString> Warnings;

    TestTrue(
        TEXT("Body profile should parse"),
        FBDFRDazBodyProfileAdapter::ParseBodyProfileJson(
            Json,
            Profile,
            &Warnings));

    TestEqual(
        TEXT("Walk and Run test clips should be imported"),
        Profile.AnimationClips.Num(),
        2);

    if (Profile.AnimationClips.Num() == 2)
    {
        TestEqual(
            TEXT("First animation clip should be Walk"),
            Profile.AnimationClips[0].Type,
            EBDFRBodyAnimationClipType::Walk);

        TestEqual(
            TEXT("Second animation clip should be Run"),
            Profile.AnimationClips[1].Type,
            EBDFRBodyAnimationClipType::Run);

        TestTrue(
            TEXT("Walk clip should preserve looping"),
            Profile.AnimationClips[0].bLoop);
    }

    TestEqual(
        TEXT("One region should be imported"),
        Profile.Regions.Num(),
        1);

    if (Profile.Regions.Num() == 1)
    {
        const FBDFRBodyRegion& Region = Profile.Regions[0];

        TestEqual(
            TEXT("Region type should be Muscle"),
            Region.Type,
            EBDFRBodyRegionType::Muscle);

        TestEqual(
            TEXT("Anchor A should be preserved"),
            Region.AnchorA,
            FName(TEXT("l_upperarm")));

        TestEqual(
            TEXT("Anchor B should be preserved"),
            Region.AnchorB,
            FName(TEXT("l_forearm")));

        TestTrue(
            TEXT("Radius should be preserved"),
            Region.RadiusCm.Equals(
                FVector(3.0, 2.0, 8.0),
                KINDA_SMALL_NUMBER));
    }

    return true;
}

#endif
