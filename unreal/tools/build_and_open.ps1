#requires -Version 5.1
<#
One-click LOCAL Windows build. Builds authentic recovered Lua 5.0 bridge
and the Unreal Engine game/editor modules. Never bundles original retail data.
#>
[CmdletBinding()]
param(
    [string]$UnrealEngineRoot = $env:UE_5_6_ROOT,
    [switch]$SkipSDKBootstrap,
    [switch]$SkipEditorLaunch
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function MustRun {
    param([string]$Program, [string[]]$Arguments, [string]$Failure)
    Write-Host "RUN: $Program $($Arguments -join ' ')" -ForegroundColor Cyan
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Failure (exit code $LASTEXITCODE)"
    }
}

$UnrealDir = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$Repo = (Resolve-Path (Join-Path $UnrealDir "..")).Path
$Native = Join-Path $Repo "native"
$NativeLuaProject = Join-Path $Native "unreal_bridge"
$BuildDir = Join-Path $Repo "native\build-unreal-lua-standalone"
$ProjectFile = Join-Path $UnrealDir "Getchu3.uproject"
if (!(Test-Path -LiteralPath $ProjectFile -PathType Leaf)) {
    throw "The actual Unreal project is missing: $ProjectFile"
}
if (!(Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake is required for the original Lua 5.0 host runtime."
}
if (-not $UnrealEngineRoot) {
    foreach ($Candidate in @("$env:ProgramFiles\Epic Games\UE_5.6",
                              "C:\Program Files\Epic Games\UE_5.6")) {
        if (Test-Path -LiteralPath (Join-Path $Candidate "Engine\Build\BatchFiles\Build.bat")) {
            $UnrealEngineRoot = $Candidate
            break
        }
    }
}
if (!$UnrealEngineRoot) {
    throw "Unreal Engine 5.6 not found. Set UE_5_6_ROOT or pass -UnrealEngineRoot."
}
$EditorBuild = Join-Path $UnrealEngineRoot "Engine\Build\BatchFiles\Build.bat"
$EditorBinary = Join-Path $UnrealEngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"
if (!(Test-Path -LiteralPath $EditorBuild -PathType Leaf)) {
    throw "Not an Unreal Engine 5.6 installation: $EditorBuild"
}
if (!(Test-Path -LiteralPath $EditorBinary -PathType Leaf)) {
    throw "UnrealEditor.exe missing: $EditorBinary"
}

# Pinned MIT ae3-sdk tools, installed to an owner-local virtual environment.
# A commit-specific ZIP avoids requiring a separate Git command-line install.
# No original PlayStation data is fetched or distributed by this step.
if (-not $SkipSDKBootstrap) {
    $Venv = Join-Path $Repo ".getchu3\ae3sdk-venv"
    $SdkPython = Join-Path $Venv "Scripts\python.exe"
    $SdkCli = Join-Path $Venv "Scripts\ae3.exe"
    if (!(Test-Path -LiteralPath $SdkCli -PathType Leaf)) {
        if (Get-Command py -ErrorAction SilentlyContinue) {
            MustRun "py" @("-3", "-m", "venv", $Venv) "Failed to create Python environment"
        } elseif (Get-Command python -ErrorAction SilentlyContinue) {
            MustRun "python" @("-m", "venv", $Venv) "Failed to create Python environment"
        } else {
            throw "Python 3 is needed to install the MIT ae3-sdk resource converter."
        }
        $PinnedTools = "https://github.com/pxdl/ae3-sdk/archive/5b2207e3580ae4cc1d588641f4fe3f898350e501.zip#subdirectory=tools"
        MustRun $SdkPython @("-m", "pip", "install", "--disable-pip-version-check",
                            "Pillow", $PinnedTools) "Pinned MIT ae3-sdk installation failed"
        if (!(Test-Path -LiteralPath $SdkCli -PathType Leaf)) {
            throw "Pinned ae3-sdk did not create its ae3 converter."
        }
    }
    $env:PATH = (Join-Path $Venv "Scripts") + [IO.Path]::PathSeparator + $env:PATH
}
# Importer regressions are mandatory. A source build must not pass while the
# ISO extractor/media adapters reject all valid input or accept fake game data.
$TestPython = if (Test-Path -LiteralPath (Join-Path $Repo ".getchu3\ae3sdk-venv\Scripts\python.exe")) {
    Join-Path $Repo ".getchu3\ae3sdk-venv\Scripts\python.exe"
} elseif (Get-Command py -ErrorAction SilentlyContinue) {
    "py"
} elseif (Get-Command python -ErrorAction SilentlyContinue) {
    "python"
} else {
    throw "Python 3 is required for original ISO importer verification."
}
$PyPrefix = if ($TestPython -eq "py") { @("-3") } else { @() }
MustRun $TestPython ($PyPrefix + @("-c",
    "import sys; assert sys.version_info >= (3,10), 'Original importer requires Python 3.10 or later'")) `
    "Python 3.10+ is required for native stage PCK recovery and Unreal importer"
MustRun $TestPython ($PyPrefix + @("-m", "unittest", "discover", "-s",
                                  (Join-Path $UnrealDir "tests"), "-v")) `
    "Original ISO/asset/Lua binding importer regressions failed"
MustRun $TestPython ($PyPrefix + @("-m", "unittest", "discover", "-s",
                                  (Join-Path $Native "tools"), "-p",
                                  "test_unpack_original_stage_pck.py", "-v")) `
    "Original PCK member source extraction or collision identity tests failed"

MustRun "cmake" @("-S", $NativeLuaProject, "-B", $BuildDir, "-G", "Visual Studio 17 2022",
                  "-A", "x64") "Native Lua 5.0 bridge configuration failed"
MustRun "cmake" @("--build", $BuildDir, "--config", "Release",
                  "--target", "Getchu3OriginalLuaBridge", "Getchu3OriginalUnrealLuaBridgeTests",
                  "Getchu3OriginalLuaStageBindingTests",
                  "Getchu3OriginalHealthBridgeTests",
                  "Getchu3OriginalPlayerResourceTests",
                  "Getchu3OriginalManagerChildTests",
                  "Getchu3OriginalManagerInitializerTests",
                  "Getchu3OriginalManagerHostAbiTests",
                  "Getchu3OriginalActorRouteHostAbiTests",
                  "Getchu3OriginalPolygonRayHostAbiTests",
                  "Getchu3OriginalActorRouteRaycastV2Tests",
                  "Getchu3OriginalActorTargetAbiTests",
                  "Getchu3OriginalRouteNativeTargetV3Tests",
                  "Getchu3OriginalWindowsSha256Tests",
                  "--parallel") "Native Lua 5.0 bridge compilation failed"
MustRun "ctest" @("--test-dir", $BuildDir, "-C", "Release",
                  "-R", "^(OriginalLua50UnrealHostBridgeFailClosed|OriginalLua50HostStageBindings|OriginalChangeHPSourceStateReadWrite|OriginalPlayerResourceVirtual2|OriginalManagerChildCtorLifecycle|OriginalManagerInitializer57BCB8|OriginalManagerInitializerHostAbi|OriginalActorRoute57C7CCHostAbi|OriginalPolygonRaycast4C1B3CHostAbi|OriginalRoute57C7CCNativeRaycast4C1B3C|OriginalActorTarget57C8ECNativeFastAndRandom|OriginalRoute57C7CCTarget57C8ECNativeCommit|OriginalWindowsSHA256SourceVectors)$",
                  "--output-on-failure") "Native Lua 5.0 bridge tests failed"

$OriginalDll = Join-Path $BuildDir "Release\Getchu3OriginalLuaBridge.dll"
if (!(Test-Path -LiteralPath $OriginalDll -PathType Leaf)) {
    throw "Original native Lua bridge DLL missing: $OriginalDll"
}
$RuntimeDir = Join-Path $UnrealDir "Binaries\Win64"
New-Item -ItemType Directory -Path $RuntimeDir -Force | Out-Null
Copy-Item -LiteralPath $OriginalDll -Destination (Join-Path $RuntimeDir "Getchu3OriginalLuaBridge.dll") -Force

MustRun $EditorBuild @("Getchu3Editor", "Win64", "Development",
                      "-Project=$ProjectFile", "-WaitMutex", "-NoHotReload") "Unreal project compilation failed"

Write-Host "Native source bridge and Unreal project compiled. Game playability NOT established." -ForegroundColor Yellow
if (-not $SkipEditorLaunch) {
    Start-Process -FilePath $EditorBinary -ArgumentList ('"' + $ProjectFile + '"')
}
