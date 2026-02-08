using UnrealBuildTool;
using System.Collections.Generic;

public class MyProjectTarget : TargetRules
{
    public MyProjectTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        CppStandard = CppStandardVersion.Cpp20;
        ExtraModuleNames.Add("ProjectVega");
    }
}
