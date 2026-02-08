using UnrealBuildTool;
using System.Collections.Generic;

public class MyProjectEditorTarget : TargetRules
{
    public MyProjectEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        // Use C++20 to match engine headers
        CppStandard = CppStandardVersion.Cpp20;
        // Allow this target to override shared build environment settings when using an installed engine
        bOverrideBuildEnvironment = true;
        ExtraModuleNames.Add("ProjectVega");
    }
}
