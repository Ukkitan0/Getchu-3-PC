using UnrealBuildTool;
using System.Collections.Generic;

public class Getchu3Target : TargetRules
{
    public Getchu3Target(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("Getchu3");
    }
}
