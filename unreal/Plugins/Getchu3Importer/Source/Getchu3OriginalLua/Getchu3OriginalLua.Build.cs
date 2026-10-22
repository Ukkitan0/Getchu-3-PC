using UnrealBuildTool;
using System.IO;

public class Getchu3OriginalLua : ModuleRules
{
    public Getchu3OriginalLua(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine"
        });
        PrivateDependencyModuleNames.AddRange(new[] { "Projects" });

        // Use the SAME native bridge C ABI header as the original Lua 5.0 DLL.
        // No copied or alternative ScriptValue representation in Unreal.
        string NativeInclude = Path.GetFullPath(Path.Combine(ModuleDirectory,
            "..", "..", "..", "..", "..", "native", "include"));
        if (!Directory.Exists(NativeInclude))
            throw new BuildException("Original native Lua bridge headers are missing from this checkout: " + NativeInclude);
        PrivateIncludePaths.Add(NativeInclude);
    }
}
