using UnrealBuildTool;

public class OpenWorldTesterEditor : ModuleRules
{
    public OpenWorldTesterEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
        {
                "Core",
                "EditorSubsystem",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "AssetTools",
                "EditorFramework",
				"UnrealEd",
                "OpenWorldTester",
                
                
            }
        );
    }
}