# Build the native C/C++ worktree on Windows without dispatching GitHub Actions.
# This script does not claim the original game is bootable: --retail deliberately
# fails closed until authentic original engine startup/frame dispatch is integrated.
[CmdletBinding()]
param(
    [ValidateSet('Debug','Release','RelWithDebInfo')]
    [string]$Configuration = 'Release',
    [string]$BuildDirectory = '',
    [switch]$SkipTests,
    [switch]$RunRetail,
    [string]$OriginalIso = '',
    [string]$Stage = '',
    [string]$Extracted = '',
    [string]$Glb = '',
    [string]$Spawn = 'spawn_default',
    [string]$LocalLua50 = '',
    [string]$LocalCgltf = ''
)
$ErrorActionPreference = 'Stop'
$native = (Resolve-Path (Join-Path $PSScriptRoot '.')).Path
if (-not $BuildDirectory) {
    $BuildDirectory = Join-Path $native 'build-windows-native'
}
$BuildDirectory = [System.IO.Path]::GetFullPath($BuildDirectory)

function Assert-Success([string]$Step) {
    if ($LASTEXITCODE -ne 0) { throw "$Step failed with exit code $LASTEXITCODE" }
}

try {
    $cmake = Get-Command 'cmake.exe' -ErrorAction Stop
    $configure = @('-S', $native, '-B', $BuildDirectory,
                   "-DCMAKE_BUILD_TYPE=$Configuration")
    if ($LocalLua50) {
        if (-not (Test-Path (Join-Path $LocalLua50 'src/lapi.c'))) {
            throw 'LocalLua50 must contain the original unmodified Lua 5.0.3 source tree'
        }
        $configure += "-DFETCHCONTENT_SOURCE_DIR_LUA50=$LocalLua50"
    }
    if ($LocalCgltf) {
        if (-not (Test-Path (Join-Path $LocalCgltf 'cgltf.h'))) {
            throw 'LocalCgltf must contain the locally cached cgltf.h dependency'
        }
        $configure += "-DFETCHCONTENT_SOURCE_DIR_GETCHUCGLTF=$LocalCgltf"
    }

    Write-Host '[1/3] Configure authentic native C/C++ source (not an emulator)'
    & $cmake.Source @configure
    Assert-Success 'CMake configure'

    Write-Host '[2/3] Build full native target graph'
    & $cmake.Source --build $BuildDirectory --config $Configuration --parallel
    Assert-Success 'CMake build'

    if (-not $SkipTests) {
        Write-Host '[3/3] Run native component CTest suite (NOT gameplay verification)'
        & ctest.exe --test-dir $BuildDirectory -C $Configuration --output-on-failure
        Assert-Success 'CTest'
    } else {
        Write-Host '[3/3] CTest explicitly skipped with -SkipTests.'
    }

    $candidates = @(
        (Join-Path $BuildDirectory "$Configuration\Getchu3Native.exe"),
        (Join-Path $BuildDirectory 'Getchu3Native.exe')
    )
    $exe = $candidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
        Select-Object -First 1
    if (-not $exe) {
        throw 'Native Win32 application was not produced; inspect the CMake target configuration.'
    }
    Write-Host "NATIVE WINDOWS BUILD: $exe"
    Write-Host 'ENGINE STATUS: original Player/Monkey startup and frame dispatch NOT yet integrated.'

    if ($RunRetail) {
        if (-not ($OriginalIso -and $Stage -and $Extracted -and $Glb)) {
            throw 'Real retail-stage attempt requires -OriginalIso -Stage -Extracted -Glb'
        }
        $python = Get-Command 'python.exe' -ErrorAction Stop
        $launcher = Join-Path $native 'tools/launch_owned_retail_stage.py'
        Write-Host 'Attempting original user-owned retail stage (no fake game-scene fallback)'
        $launchArgs = @($launcher, '--iso', $OriginalIso, '--stage', $Stage,
                        '--extracted', $Extracted, '--glb', $Glb,
                        '--exe', $exe, '--spawn', $Spawn)
        & $python.Source @launchArgs
        Assert-Success 'Retail-stage/native-original-engine attempt'
    }
    exit 0
}
catch {
    Write-Error "NATIVE BUILD OR ORIGINAL ENGINE BOOT BLOCKED: $($_.Exception.Message)"
    exit 1
}
