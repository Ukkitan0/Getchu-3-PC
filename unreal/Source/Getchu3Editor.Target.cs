using UnrealBuildTool;
using System.Collections.Generic;

public class Getchu3EditorTarget : TargetRules
{
    public Getchu3EditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("Getchu3");
    }
}
