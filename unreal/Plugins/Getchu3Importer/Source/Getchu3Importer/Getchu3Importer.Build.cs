using UnrealBuildTool;

public class Getchu3Importer : ModuleRules
{
    public Getchu3Importer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "Slate", "SlateCore", "ToolMenus",
            "LevelEditor", "DesktopPlatform", "Projects", "Json", "PythonScriptPlugin"
        });
    }
}
