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
        PrivateDependencyModuleNames.AddRange(new[] { "Projects", "Json" });

        // Use the SAME native bridge C ABI header as the original Lua 5.0 DLL.
        // No copied or alternative ScriptValue representation in Unreal.
        string NativeInclude = Path.GetFullPath(Path.Combine(ModuleDirectory,
            "..", "..", "..", "..", "..", "native", "include"));
        if (!Directory.Exists(NativeInclude))
            throw new BuildException("Original native Lua bridge headers are missing from this checkout: " + NativeInclude);
        PrivateIncludePaths.Add(NativeInclude);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // The Win64 SHA-256 verifier uses CNG BCryptHash. Link the
            // Windows SDK system import library, not Unreal's fatal SHA stub.
            PublicAdditionalLibraries.Add("bcrypt.lib");
            string BridgeDll = Path.GetFullPath(Path.Combine(ModuleDirectory,
                "..", "..", "..", "..", "Binaries", "Win64",
                "Getchu3OriginalLuaBridge.dll"));
            if (!File.Exists(BridgeDll))
                throw new BuildException("Original Lua 5.0 DLL missing; run unreal\\BuildAndOpen.bat before building. Expected: " + BridgeDll);
            RuntimeDependencies.Add("$(TargetOutputDir)/Getchu3OriginalLuaBridge.dll",
                BridgeDll, StagedFileType.NonUFS);
        }
    }
}
