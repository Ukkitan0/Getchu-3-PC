# Native Windows C++ PS2 recompilation bridge. No WSL or browser substitutions.
# Source is external MIT code; this project never downloads game data.
[CmdletBinding()]
param(
    [string]$Elf = "",
    [string]$Disc = "",
    [string]$Workspace = "",
    [switch]$Run,
    [ValidateSet('constructors','common','all')][string]$SeedMode = 'constructors',
    [switch]$ReuseUpstream
)
$ErrorActionPreference = 'Stop'
$Project = Split-Path -Parent $MyInvocation.MyCommand.Path
$ExpectedHash = '5A8A73CF741B2E923F652F9BDF35D1798471FF31FB960B63ED010A78D1AC2D63'
$PinnedUpstream = '6526f4803d98e10de50bea3af104a4d26e9df033'
if (-not $Workspace) { $Workspace = Join-Path $Project 'fastlane\private' }
New-Item -ItemType Directory -Path $Workspace -Force | Out-Null
$Log = Join-Path $Workspace 'build.log'
"Getchu-3-PC native AOT build, $(Get-Date -Format o)" | Set-Content -Path $Log
function Fail([string]$Reason) { throw $Reason }
function RunProcess([string]$File, [string[]]$Arg) {
    "`n> $File $($Arg -join ' ')" | Out-File -FilePath $Log -Append
    $PreviousPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue' # native stderr is diagnostic output in PS 5.1
    try {
        & $File @Arg 2>&1 | ForEach-Object { "$_" | Tee-Object -FilePath $Log -Append }
        $ResultCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $PreviousPreference
    }
    if ($ResultCode -ne 0) { Fail "Command failed ($ResultCode): $File. See $Log" }
}
foreach ($Name in @('git', 'cmake', 'py')) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        Fail "Missing prerequisite: $Name. Install Git, CMake, Python 3 and Visual Studio 2022 with the C++ workload."
    }
}
if (-not $Elf) {
    $Places = @(
        'C:\ApeEscape3_OneClick\game\original\SCUS_975.01',
        'C:\AE3Recompile2\WebRecompNative\upstream\games\ae3\disc\SCUS_975.01'
    )
    $Elf = $Places | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
}
if (-not $Elf -or -not (Test-Path -LiteralPath $Elf -PathType Leaf)) {
    Fail 'Original US SCUS_975.01 ELF not found. Run with -Elf "C:\path\SCUS_975.01" (must come from your own game).'
}
$Elf = (Resolve-Path -LiteralPath $Elf).Path
$ActualHash = (Get-FileHash -LiteralPath $Elf -Algorithm SHA256).Hash
if ($ActualHash -ne $ExpectedHash) {
    Fail "Wrong retail ELF version: $ActualHash. This pipeline is pinned to US SCUS_975.01 $ExpectedHash."
}
$Upstream = Join-Path $Workspace 'ps2-web-recomp'
if (-not (Test-Path -LiteralPath (Join-Path $Upstream '.git'))) {
    RunProcess 'git' @('clone', 'https://github.com/snuri00/ps2-web-recomp.git', $Upstream)
    RunProcess 'git' @('-C', $Upstream, 'checkout', '--detach', $PinnedUpstream)
} else {
    $Installed = (& git -C $Upstream rev-parse HEAD).Trim()
    if ($Installed -ne $PinnedUpstream -and -not $ReuseUpstream) {
        Fail "Upstream is $Installed, expected $PinnedUpstream. Refusing to overwrite installed source; select a fresh -Workspace or pass -ReuseUpstream explicitly."
    }
}
$ToolBuild = Join-Path $Workspace 'build-tools'
# This is the current upstream CMake target, not a guessed project name.
RunProcess 'cmake' @('-S', $Upstream, '-B', $ToolBuild, '-G', 'Visual Studio 17 2022', '-A', 'x64')
RunProcess 'cmake' @('--build', $ToolBuild, '--config', 'Release', '--target', 'eerecomp', '--parallel')
$Compiler = Join-Path $ToolBuild 'Release\eerecomp.exe'
if (-not (Test-Path -LiteralPath $Compiler)) { Fail "Missing built native EE recompiler: $Compiler" }
$Entries = Join-Path $Workspace 'entries'
$Generated = Join-Path $Workspace 'generated\ae3'
RunProcess 'py' @('-3', (Join-Path $Project 'tools\bulk_recompile.py'), '--elf', $Elf, '--out', $Entries, '--seed-mode', $SeedMode, '--eerecomp', $Compiler, '--cpp-out', $Generated)
$NativeBuild = Join-Path $Workspace 'build-native-ae3'
$Args = @('-S', $Upstream, '-B', $NativeBuild, '-G', 'Visual Studio 17 2022', '-A', 'x64', "-DPS2_GENERATED=$Generated")
if ($env:VCPKG_ROOT -and (Test-Path -LiteralPath (Join-Path $env:VCPKG_ROOT 'scripts\buildsystems\vcpkg.cmake'))) {
    $Args += "-DCMAKE_TOOLCHAIN_FILE=$(Join-Path $env:VCPKG_ROOT 'scripts\buildsystems\vcpkg.cmake')"
}
RunProcess 'cmake' $Args
RunProcess 'cmake' @('--build', $NativeBuild, '--config', 'Release', '--target', 'ps2run', '--parallel')
$Runner = Join-Path $NativeBuild 'Release\ps2run.exe'
if (-not (Test-Path -LiteralPath $Runner)) { Fail "Native runner not found after build: $Runner" }
Write-Host "Compiled native Windows runner: $Runner"
Write-Host "NOTE: No title-screen or gameplay claim; Windows compatibility is not validated here."
Write-Host "No browser/WSL frontend has been substituted. Original game files stay local."
if ($Run) {
    if (-not $Disc -or -not (Test-Path -LiteralPath $Disc -PathType Leaf)) {
        Fail 'To run the game, supply your own ISO using -Disc "C:\path\Ape Escape 3 (USA).iso".'
    }
    # SDL2 is optional in upstream CMake; --window is available only when built with SDL2.
    RunProcess $Runner @($Elf, '--disc', (Resolve-Path -LiteralPath $Disc).Path, '--window', '--trace-syscalls')
}