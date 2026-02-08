using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectVega : ModuleRules
{
    public ProjectVega(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "GameplayTags", "UMG", "Slate", "SlateCore", "AIModule" });

        PrivateDependencyModuleNames.AddRange(new string[] {  });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }
    }
}
