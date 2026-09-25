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
