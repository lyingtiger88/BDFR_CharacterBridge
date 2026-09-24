#if WITH_DEV_AUTOMATION_TESTS

#include "Adapters/Daz/BDFRDazMorphAdapter.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRDazMorphTransferPlanTest,
    "BDFR.CharacterBridge.Daz.Morph.BuildTransferPlan",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBDFRDazMorphTransferPlanTest::RunTest(const FString& Parameters)
{
    const FString Json = TEXT(R"JSON(
    {
        "Morphs": [
            { "Name": "Genesis8Female.FBMBodybuilder", "Label": "Bodybuilder" },
            { "Name": "eCTRLSmile", "Label": "Smile" },
            { "Name": "pJCMShldrUp_90_L", "Label": "Shoulder Corrective" }
        ],
        "JointLinks": [
            {
                "Bone": "lShldrBend",
                "Morph": "pJCMShldrUp_90_L",
                "Axis": "ZRotate",
                "Scalar": 0.011111,
                "Alpha": 1.0,
                "Keys": [
                    { "Angle": 0.0, "Value": 0.0 },
                    { "Angle": 90.0, "Value": 1.0 }
                ]
            }
        ]
    }
    )JSON");

    const TArray<FName> ImportedMorphs =
    {
        TEXT("FBMBodybuilder"),
        TEXT("eCTRLSmile"),
        TEXT("pJCMShldrUp_90_L"),
        TEXT("CustomExtraMorph")
    };

    FBDFRMorphTransferPlan Plan;
    const bool bBuilt =
        FBDFRDazMorphAdapter::BuildTransferPlanFromDtuJson(
            Json,
            ImportedMorphs,
            Plan,
            false);

    TestTrue(TEXT("DTU morph transfer plan should parse"), bBuilt);
    TestEqual(TEXT("No requested morph should be missing"), Plan.MissingRequestedMorphs.Num(), 0);
    TestEqual(TEXT("One joint-driven link should be parsed"), Plan.JointDrivenLinks.Num(), 1);
    TestTrue(TEXT("Unlisted imported morph should be preserved"), Plan.UnlistedImportedMorphs.Contains(FName(TEXT("CustomExtraMorph"))));

    const FBDFRMorphDescriptor* BodyMorph = Plan.Morphs.FindByPredicate(
        [](const FBDFRMorphDescriptor& Morph)
        {
            return Morph.SourceInternalName == FName(TEXT("FBMBodybuilder"));
        });

    TestNotNull(TEXT("Body morph should exist"), BodyMorph);
    if (BodyMorph)
    {
        TestEqual(TEXT("Body morph should classify as body"), BodyMorph->Semantic, EBDFRMorphSemantic::Body);
        TestEqual(TEXT("Display label should be used as target name"), BodyMorph->TargetMorphName, FName(TEXT("Bodybuilder")));
    }

    const FBDFRMorphDescriptor* JcmMorph = Plan.Morphs.FindByPredicate(
        [](const FBDFRMorphDescriptor& Morph)
        {
            return Morph.SourceInternalName == FName(TEXT("pJCMShldrUp_90_L"));
        });

    TestNotNull(TEXT("JCM morph should exist"), JcmMorph);
    if (JcmMorph)
    {
        TestTrue(TEXT("JCM morph should be marked joint-driven"), JcmMorph->bJointDriven);
        TestEqual(TEXT("JCM morph should classify as corrective"), JcmMorph->Semantic, EBDFRMorphSemantic::Corrective);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBDFRDazMorphNormalizationTest,
    "BDFR.CharacterBridge.Daz.Morph.NormalizeNames",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBDFRDazMorphNormalizationTest::RunTest(const FString& Parameters)
{
    TestEqual(
        TEXT("Daz internal prefix before a period should be removed"),
        FBDFRDazMorphAdapter::NormalizeDazMorphName(TEXT("Genesis8Female.FBMBodybuilder")),
        FName(TEXT("FBMBodybuilder")));

    TestEqual(
        TEXT("FBX object namespace should be removed"),
        FBDFRDazMorphAdapter::NormalizeFbxMorphChannelName(TEXT("Genesis8Female__eCTRLSmile")),
        FName(TEXT("eCTRLSmile")));

    return true;
}

#endif
