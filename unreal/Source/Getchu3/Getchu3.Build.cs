using UnrealBuildTool;

public class Getchu3 : ModuleRules
{
    public Getchu3(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new[] { "Getchu3OriginalLua" });
    }
}
