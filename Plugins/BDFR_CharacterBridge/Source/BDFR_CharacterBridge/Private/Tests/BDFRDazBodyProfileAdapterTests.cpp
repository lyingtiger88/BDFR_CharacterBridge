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


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRDazNativeBodyProfileParseTest,
    "BDFR.CharacterBridge.Daz.BodyProfile.NativeV3",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FBDFRDazNativeBodyProfileParseTest::RunTest(
    const FString& Parameters)
{
    const FString Json = TEXT(R"JSON(
    {
        "schema": "BDFR.BodyProfile",
        "version": 3,
        "units": "cm",
        "coordinateSystem": "DazStudio",
        "characterName": "Genesis9Female",
        "gender": "Female",
        "genesisProfile": "Genesis 9",
        "animationClips": [
            {
                "name": "Walk (Default)",
                "type": "Walk",
                "sourceFile": "",
                "loop": true
            }
        ],
        "regions": [
            {
                "id": "BDFR_Breast_Left_420_305",
                "name": "Breast_L",
                "type": "Breast",
                "side": "Left",
                "anchorA": "l_shoulder",
                "anchorB": "l_upperarm",
                "normalizedX": 0.42,
                "normalizedY": 0.305,
                "radiusXcm": 6.0,
                "radiusYcm": 5.0,
                "radiusZcm": 4.0,
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
        TEXT("Native v3 body profile should parse"),
        FBDFRDazBodyProfileAdapter::ParseBodyProfileJson(
            Json,
            Profile,
            &Warnings));

    TestEqual(TEXT("Version should be 3"), Profile.Version, 3);
    TestEqual(TEXT("Gender should be Female"), Profile.Gender, FString(TEXT("Female")));
    TestEqual(TEXT("Genesis profile should be preserved"), Profile.GenesisProfile, FString(TEXT("Genesis 9")));
    TestEqual(TEXT("One region should be imported"), Profile.Regions.Num(), 1);

    if (Profile.Regions.Num() == 1)
    {
        const FBDFRBodyRegion& Region = Profile.Regions[0];
        TestEqual(TEXT("Region should be Breast"), Region.Type, EBDFRBodyRegionType::Breast);
        TestTrue(
            TEXT("Normalized body-map point should be preserved"),
            Region.NormalizedBodyMapPosition.Equals(
                FVector2D(0.42f, 0.305f),
                KINDA_SMALL_NUMBER));
        TestTrue(
            TEXT("Split radius fields should become RadiusCm"),
            Region.RadiusCm.Equals(
                FVector(6.0f, 5.0f, 4.0f),
                KINDA_SMALL_NUMBER));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRDazEmbeddedDtuBodyProfileParseTest,
    "BDFR.CharacterBridge.Daz.BodyProfile.EmbeddedDTU",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)

bool FBDFRDazEmbeddedDtuBodyProfileParseTest::RunTest(
    const FString& Parameters)
{
    const FString DtuJson = TEXT(R"JSON(
    {
        "Asset Name": "Genesis9Female",
        "Asset Type": "SkeletalMesh",
        "BDFRBodyProfile": {
            "version": 3,
            "units": "cm",
            "coordinateSystem": "DazStudio",
            "characterName": "Genesis9Female",
            "gender": "Female",
            "genesisProfile": "Genesis 9",
            "regions": [
                {
                    "id": "BDFR_Quadriceps_Left_430_660",
                    "name": "Quadriceps_L",
                    "type": "Muscle",
                    "side": "Left",
                    "anchorA": "l_thigh",
                    "anchorB": "l_shin",
                    "normalizedX": 0.43,
                    "normalizedY": 0.66,
                    "radiusXcm": 5.5,
                    "radiusYcm": 13.0,
                    "radiusZcm": 5.5
                }
            ]
        }
    }
    )JSON");

    FBDFRBodyProfile Profile;
    TArray<FString> Warnings;

    TestTrue(
        TEXT("Embedded DTU body profile should parse"),
        FBDFRDazBodyProfileAdapter::ParseDtuBodyProfileJson(
            DtuJson,
            Profile,
            &Warnings));

    TestEqual(TEXT("Embedded profile should contain one region"), Profile.Regions.Num(), 1);
    if (Profile.Regions.Num() == 1)
    {
        TestEqual(
            TEXT("Embedded region Anchor A should be preserved"),
            Profile.Regions[0].AnchorA,
            FName(TEXT("l_thigh")));
    }

    return true;
}

#endif
