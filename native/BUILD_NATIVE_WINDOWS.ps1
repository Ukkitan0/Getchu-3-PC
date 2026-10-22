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
    [string]$LocalCgltf = '',
    [string]$OriginalElf = '',
    [switch]$RequireIntegratedWorkstreams
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
    if ($RequireIntegratedWorkstreams) {
        # Require all five new native specialist source/test sets for an
        # integration build; prevent an accidental partial-source success.
        $configure += '-DAE3_REQUIRE_PARALLEL_INTEGRATION=ON'
    }
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
        if ($OriginalElf) {
            if (-not (Test-Path -LiteralPath $OriginalElf -PathType Leaf)) {
                throw 'Private original USA SCUS_975.01 was not found'
            }
            $expectedSha = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
            $actualSha = (Get-FileHash -LiteralPath $OriginalElf -Algorithm SHA256).Hash.ToLowerInvariant()
            if ($actualSha -ne $expectedSha) {
                throw "Original SCUS_975.01 SHA-256 mismatch: expected $expectedSha, got $actualSha"
            }
            Write-Host 'Private original USA SCUS_975.01 SHA-256: VERIFIED'
        }
        $python = Get-Command 'python.exe' -ErrorAction Stop
        $launcher = Join-Path $native 'tools/launch_owned_retail_stage.py'
        Write-Host 'Attempting original user-owned retail stage (no fake game-scene fallback)'
        $launchArgs = @($launcher, '--iso', $OriginalIso, '--stage', $Stage,
                        '--extracted', $Extracted, '--glb', $Glb,
                        '--exe', $exe, '--spawn', $Spawn)
        # The GUI executable does not have a console stdout stream. Capture
        # its checkpoint journal with a UNIQUE path for this attempt, then
        # print the first actual blocked subsystem even on failure.
        $bootLog = Join-Path $BuildDirectory (
            'retail-native-boot-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fffffff') + '.log')
        $priorBootLog = [Environment]::GetEnvironmentVariable('AE3_NATIVE_BOOT_LOG','Process')
        try {
            $env:AE3_NATIVE_BOOT_LOG = $bootLog
            & $python.Source @launchArgs
            $launchExit = $LASTEXITCODE
        }
        finally {
            [Environment]::SetEnvironmentVariable('AE3_NATIVE_BOOT_LOG',$priorBootLog,'Process')
        }
        if (Test-Path -LiteralPath $bootLog -PathType Leaf) {
            Write-Host "NATIVE GAME STARTUP CHECKPOINTS: $bootLog"
            $steps = @(Get-Content -LiteralPath $bootLog)
            $steps | ForEach-Object { Write-Host $_ }
            $firstBlocked = $steps | Where-Object { $_ -like '*|BLOCKED|*' } |
                Select-Object -First 1
            if ($firstBlocked) { Write-Host "FIRST ORIGINAL ENGINE BLOCKER: $firstBlocked" }
        }
        else {
            Write-Host 'No native-process checkpoint file: resource preflight or Win32 initialization blocked before logging.'
        }
        if ($launchExit -ne 0) {
            throw "Retail original-engine boot blocked (launch code $launchExit). Not playable."
        }
    }
    exit 0
}
catch {
    Write-Error "NATIVE BUILD OR ORIGINAL ENGINE BOOT BLOCKED: $($_.Exception.Message)"
    exit 1
}
