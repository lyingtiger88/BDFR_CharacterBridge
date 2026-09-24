using UnrealBuildTool;

public class BDFR_CharacterBridgeEditor : ModuleRules
{
    public BDFR_CharacterBridgeEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "BDFR_CharacterBridge"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "AssetTools",
                "UnrealEd",
                "IKRig",
                "IKRigEditor"
            }
        );
    }
}
