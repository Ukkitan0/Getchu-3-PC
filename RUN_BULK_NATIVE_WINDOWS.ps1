# Native Windows x64 AOT PS2 pipeline. Original commercial game files never leave the owner's PC.
# Uses a pinned external MIT recompiler and a strictly anchor-checked Win32 overlay.
[CmdletBinding()]
param(
    [string]$Elf = "", [string]$Disc = "", [string]$Workspace = "",
    [switch]$Run,
    [ValidateSet('constructors','common','all')][string]$SeedMode = 'constructors',
    [switch]$ReuseUpstream,
    [ValidateRange(0,64)][int]$ExploreRounds = 0,
    [ValidateRange(60,20000)][int]$ExploreFrames = 1800,
    [string]$WgpuDir = "",
    [string]$VcpkgRoot = "",
    [switch]$SoftwareRenderer
)
$ErrorActionPreference = 'Stop'
$Project = Split-Path -Parent $MyInvocation.MyCommand.Path
$ExpectedHash = '5A8A73CF741B2E923F652F9BDF35D1798471FF31FB960B63ED010A78D1AC2D63'
$PinnedUpstream = '6526f4803d98e10de50bea3af104a4d26e9df033'
$PinnedVcpkg = '664933b8d385dfc01f4e0cdff4c7c74e71f6a740' # Oct 18 2026
if (-not $Workspace) { $Workspace = Join-Path $Project 'fastlane\private' }
New-Item -ItemType Directory -Path $Workspace -Force | Out-Null
$Log = Join-Path $Workspace 'build.log'
"Native recompilation pipeline $(Get-Date -Format o); upstream $PinnedUpstream" | Set-Content -Path $Log
function Fail([string]$Reason) { throw $Reason }
function RunProcess([string]$File, [string[]]$Arg) {
    "`n> $File $($Arg -join ' ')" | Out-File -FilePath $Log -Append
    $PreviousPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue' # PowerShell 5.1 treats native stderr as records
    try {
        & $File @Arg 2>&1 | ForEach-Object { "$_" | Tee-Object -FilePath $Log -Append }
        $ExitStatus = $LASTEXITCODE
    } finally { $ErrorActionPreference = $PreviousPreference }
    if ($ExitStatus -ne 0) { Fail "Command failed ($ExitStatus): $File. See $Log" }
}
foreach ($Name in @('git', 'cmake', 'py')) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        Fail "Missing $Name. Install Git, CMake, Python 3 and Visual Studio 2022 with ClangCL."
    }
}
if (-not $Elf) {
    $Places = @('C:\ApeEscape3_OneClick\game\original\SCUS_975.01',
                'C:\AE3Recompile2\WebRecompNative\upstream\games\ae3\disc\SCUS_975.01')
    $Elf = $Places | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
}
# A single owned ISO is enough: the pinned upstream disc reader will locate
# and extract its SYSTEM.CNF BOOT2 executable without a pre-existing ELF.
# Validate paths before provisioning any dependencies.
if ($Elf -and -not (Test-Path -LiteralPath $Elf -PathType Leaf)) {
    Fail "Executable not found: $Elf"
}
if ($Elf) {
    $Elf = (Resolve-Path -LiteralPath $Elf).Path
    $ActualHash = (Get-FileHash -LiteralPath $Elf -Algorithm SHA256).Hash
    if ($ActualHash -ne $ExpectedHash) { Fail "Wrong executable: $ActualHash, expected $ExpectedHash" }
}
if (-not $Elf -and -not $Disc) {
    Fail 'Provide -Disc "C:\path\Ape Escape 3.iso" or -Elf "C:\path\SCUS_975.01".'
}
if ($Run -or $ExploreRounds -gt 0) {
    if (-not $Disc -or -not (Test-Path -LiteralPath $Disc -PathType Leaf)) {
        Fail 'Native execution/exploration requires -Disc with a valid locally owned ISO.'
    }
}
if ($Disc) {
    if (-not (Test-Path -LiteralPath $Disc -PathType Leaf)) { Fail "Disc image not found: $Disc" }
    $Disc = (Resolve-Path -LiteralPath $Disc).Path
}
$Upstream = Join-Path $Workspace 'ps2-web-recomp'
if (-not (Test-Path -LiteralPath (Join-Path $Upstream '.git'))) {
    RunProcess 'git' @('clone','https://github.com/snuri00/ps2-web-recomp.git',$Upstream)
    RunProcess 'git' @('-C',$Upstream,'checkout','--detach',$PinnedUpstream)
} else {
    $Installed = (& git -C $Upstream rev-parse HEAD).Trim()
    if ($Installed -ne $PinnedUpstream -and -not $ReuseUpstream) {
        Fail "Upstream is $Installed, expected pinned $PinnedUpstream. Choose a fresh Workspace or explicitly -ReuseUpstream."
    }
}
# Extract the exact BOOT2 ELF and optional game modules only inside the
# gitignored private workspace; never assume a manually supplied ELF is from
# the same disc as the --disc argument.
$ModuleArgs = @()
$OverlayArgs = @()
$PrivateDisc = Join-Path $Workspace 'owned_disc'
$DiscInfo = $null
if ($Disc) {
    $PrepArgs = @('-3',(Join-Path $Project 'tools\prepare_disc.py'),
                  '--disc',$Disc,'--upstream',$Upstream,
                  '--out',$PrivateDisc,'--expected-sha',$ExpectedHash)
    if ($Elf) { $PrepArgs += @('--elf',$Elf) }
    RunProcess 'py' $PrepArgs
    $DiscInfo = Get-Content -LiteralPath (Join-Path $PrivateDisc 'disc_prepare.json') -Raw | ConvertFrom-Json
    $Elf = $DiscInfo.executable
    if ($DiscInfo.modules_dir) { $ModuleArgs = @('--modules', $DiscInfo.modules_dir) }
    if ($DiscInfo.overlays_dir) { $OverlayArgs = @('--overlays', $DiscInfo.overlays_dir) }
}
if (-not $Elf) { Fail 'No validated boot executable was selected' }
$Elf = (Resolve-Path -LiteralPath $Elf).Path
$ActualHash = (Get-FileHash -LiteralPath $Elf -Algorithm SHA256).Hash
if ($ActualHash -ne $ExpectedHash) { Fail "Wrong original retail boot ELF: $ActualHash" }
# The overlay is idempotent and anchored to reviewed upstream exact source.
$Report = Join-Path $Workspace 'windows_port_report.json'
RunProcess 'py' @('-3',(Join-Path $Project 'tools\windows_upstream_patch.py'),'--upstream',$Upstream,'--report',$Report)
# Local one-click builds must install the *same* SDL2 dependency as CI.
# Do not silently run a headless CMake configuration when vcpkg is missing.
if (-not $VcpkgRoot) {
    if ($env:VCPKG_ROOT) { $VcpkgRoot = $env:VCPKG_ROOT }
    elseif ($env:VCPKG_INSTALLATION_ROOT) { $VcpkgRoot = $env:VCPKG_INSTALLATION_ROOT }
    else { $VcpkgRoot = Join-Path $Workspace 'vcpkg' }
}
$VcpkgExe = Join-Path $VcpkgRoot 'vcpkg.exe'
$Toolchain = Join-Path $VcpkgRoot 'scripts\buildsystems\vcpkg.cmake'
if (-not (Test-Path -LiteralPath $Toolchain)) {
    $LocalBootstrap = Join-Path $Workspace 'vcpkg'
    if ($VcpkgRoot -ne $LocalBootstrap) {
        Fail "Invalid vcpkg root $VcpkgRoot; supply -VcpkgRoot with scripts\\buildsystems\\vcpkg.cmake"
    }
    if (Test-Path -LiteralPath $VcpkgRoot) {
        Fail "Incomplete vcpkg workspace at $VcpkgRoot; repair/remove that directory to retry bootstrap"
    }
    Write-Host 'Bootstrapping a private vcpkg installation for required SDL2...'
    RunProcess 'git' @('clone','--filter=blob:none','https://github.com/microsoft/vcpkg.git',$VcpkgRoot)
    RunProcess 'git' @('-C',$VcpkgRoot,'checkout','--detach',$PinnedVcpkg)
}
if (-not (Test-Path -LiteralPath $VcpkgExe)) {
    $Bootstrap = Join-Path $VcpkgRoot 'bootstrap-vcpkg.bat'
    if (-not (Test-Path -LiteralPath $Bootstrap)) { Fail "Missing vcpkg bootstrap script: $Bootstrap" }
    RunProcess $Bootstrap @('-disableMetrics')
}
if (-not (Test-Path -LiteralPath $Toolchain)) { Fail "Missing vcpkg CMake toolchain: $Toolchain" }
RunProcess $VcpkgExe @('install','sdl2:x64-windows')
$WgpuArgs = @()
if (-not $WgpuDir) { $WgpuDir = Join-Path $Upstream 'third_party\wgpu-native' }
$WgpuLib = Join-Path $WgpuDir 'lib\wgpu_native.lib'
if (-not $SoftwareRenderer -and -not (Test-Path -LiteralPath $WgpuLib)) {
    # Exact official release asset digest is verified against its GitHub release API.
    $WgpuUrl = 'https://github.com/gfx-rs/wgpu-native/releases/download/v29.0.1.1/wgpu-windows-x86_64-msvc-release.zip'
    $WgpuSha256 = '7E67D7445C42AEB85E30F88930FD8D7D83EE769E3390AEB1ADA75EBF3CF78132'
    $WgpuZip = Join-Path $Workspace 'wgpu-native-windows-x64.zip'
    Write-Host 'Fetching verified official Windows x64 wgpu-native v29.0.1.1 ...'
    Invoke-WebRequest -Uri $WgpuUrl -OutFile $WgpuZip -UseBasicParsing
    $ActualWgpu = (Get-FileHash -LiteralPath $WgpuZip -Algorithm SHA256).Hash
    if ($ActualWgpu -ne $WgpuSha256) { Fail "wgpu-native SHA-256 mismatch: $ActualWgpu" }
    New-Item -ItemType Directory -Path $WgpuDir -Force | Out-Null
    Expand-Archive -LiteralPath $WgpuZip -DestinationPath $WgpuDir -Force
    Remove-Item -LiteralPath $WgpuZip -Force
}
if ($SoftwareRenderer) {
    Write-Host 'Explicit software GS renderer mode selected.'
} else {
    if (-not (Test-Path -LiteralPath $WgpuLib)) { Fail "wgpu-native library missing after verified setup: $WgpuLib" }
    if (-not (Test-Path -LiteralPath (Join-Path $WgpuDir 'include\webgpu\webgpu.h'))) { Fail 'wgpu-native release missing webgpu.h' }
    $WgpuArgs = @("-DPS2_WGPU_DIR=$WgpuDir")
}
$ExtraCmake = @()
if ($Toolchain) { $ExtraCmake += "-DCMAKE_TOOLCHAIN_FILE=$Toolchain" }
$ToolBuild = Join-Path $Workspace 'build-tools'
RunProcess 'cmake' (@('-S',$Upstream,'-B',$ToolBuild,'-G','Visual Studio 17 2022','-A','x64','-T','ClangCL') + $ExtraCmake)
RunProcess 'cmake' @('--build',$ToolBuild,'--config','Release','--target','eerecomp','vurecomp','--parallel')
$Compiler = Join-Path $ToolBuild 'Release\eerecomp.exe'
$VuCompiler = Join-Path $ToolBuild 'Release\vurecomp.exe'
foreach($App in @($Compiler,$VuCompiler)) { if (-not (Test-Path -LiteralPath $App)) { Fail "Missing built upstream executable: $App" } }
$Entries = Join-Path $Workspace 'entries'
$Generated = Join-Path $Workspace 'generated\ae3'
$BulkArgs = @('-3',(Join-Path $Project 'tools\bulk_recompile.py'),'--elf',$Elf,
              '--out',$Entries,'--seed-mode',$SeedMode,
              '--eerecomp',$Compiler,'--cpp-out',$Generated) + $ModuleArgs + $OverlayArgs
RunProcess 'py' $BulkArgs
$EntryList = Join-Path $Entries 'entries.txt'
$TranslateArgs = @($Elf,'--entries',$EntryList) + $ModuleArgs + $OverlayArgs + @('--out',$Generated)
$AnalysisInventory = Join-Path $Project 'analysis\full_elf\ee_text_word_inventory.tsv.gz'
if (-not (Test-Path -LiteralPath $AnalysisInventory)) {
    # A fresh public checkout contains NO copyrighted retail-code inventory.
    # The bulk recompiler derives one privately from the user's verified ELF.
    $AnalysisInventory = Join-Path $Entries 'derived_from_owned_elf\ee_text_word_inventory.tsv.gz'
}
if (-not (Test-Path -LiteralPath $AnalysisInventory)) { Fail 'Private EE opcode inventory was not generated' }
$NativeBuild = Join-Path $Workspace 'build-native-ae3'
# SDL is mandatory for an interactive native Windows test, not an optional silent omission.
$CmakeArgs = @('-S',$Upstream,'-B',$NativeBuild,'-G','Visual Studio 17 2022','-A','x64','-T','ClangCL',"-DPS2_GENERATED=$Generated",'-DAE3_REQUIRE_SDL2=ON') + $ExtraCmake + $WgpuArgs
RunProcess 'cmake' $CmakeArgs
function BuildRunner() {
    RunProcess 'cmake' @('--build',$NativeBuild,'--config','Release','--target','ps2run','--parallel')
}
BuildRunner
$Runner = Join-Path $NativeBuild 'Release\ps2run.exe'
if (-not (Test-Path -LiteralPath $Runner)) { Fail "Missing native Windows executable: $Runner" }
$RunnerDir = Split-Path -Parent $Runner
# Avoid shipping a native game .exe that fails immediately from a missing
# dynamically-linked SDL2.dll. vcpkg may already copy this via applocal, but
# make the one-click output self-contained even when that integration is off.
$SDL2Dll = Join-Path $VcpkgRoot 'installed\x64-windows\bin\SDL2.dll'
if (Test-Path -LiteralPath $SDL2Dll) {
    Copy-Item -LiteralPath $SDL2Dll -Destination $RunnerDir -Force
}
# Match a release providing an import library + DLL; static-lib builds have no DLL.
if (-not $SoftwareRenderer) {
    $WgpuDll = Join-Path $WgpuDir 'lib\wgpu_native.dll'
    if (Test-Path -LiteralPath $WgpuDll) {
        Copy-Item -LiteralPath $WgpuDll -Destination (Split-Path -Parent $Runner) -Force
    }
}
if ($ExploreRounds -gt 0) {
    if (-not $Disc -or -not (Test-Path -LiteralPath $Disc -PathType Leaf)) {
        Fail 'Exploration requires -Disc with the ISO you own.'
    }
    $Disc = (Resolve-Path -LiteralPath $Disc).Path
    $Misses = Join-Path $Workspace 'misses.txt'
    $VuPrograms = Join-Path $Workspace 'vu1'
    New-Item -ItemType Directory -Path $VuPrograms -Force | Out-Null
    $OldVu = $env:PS2_VU_RECORD
    $OldGpu = $env:PS2_GPU_LOW_POWER
    try {
        $env:PS2_VU_RECORD = $VuPrograms
        $env:PS2_GPU_LOW_POWER = '1'
        for ($Round=1; $Round -le $ExploreRounds; $Round++) {
            Remove-Item -LiteralPath $Misses -ErrorAction SilentlyContinue
            "`n== exploration round $Round ==" | Tee-Object -FilePath $Log -Append
            # A diagnostic may stop intentionally on missing code; capture and continue only after validating misses.
            $OldPreference = $ErrorActionPreference
            $ErrorActionPreference = 'Continue'
            try {
                & $Runner $Elf '--disc' $Disc '--frames' "$ExploreFrames" '--misses' $Misses '--press' '300:start' '--press' '600:cross' 2>&1 | ForEach-Object { "$_" | Out-File -FilePath $Log -Append }
                $ExploreExit = $LASTEXITCODE
            } finally { $ErrorActionPreference = $OldPreference }
            "explore process code: $ExploreExit" | Out-File -FilePath $Log -Append
            RunProcess 'py' @('-3',(Join-Path $Project 'tools\native_explore.py'),'--entries',$EntryList,'--misses',$Misses,'--inventory',$AnalysisInventory,'--report',(Join-Path $Workspace 'explore_last.json'))
            $Summary = Get-Content -LiteralPath (Join-Path $Workspace 'explore_last.json') -Raw | ConvertFrom-Json
            if (-not $Summary.changed) { break }
            RunProcess $Compiler @($Elf,'--entries',$EntryList,'--out',$Generated)
            BuildRunner
        }
        if ((Get-ChildItem -LiteralPath $VuPrograms -Filter '*.bin' -File | Measure-Object).Count -gt 0) {
            RunProcess $VuCompiler @($VuPrograms,$Generated)
            BuildRunner
        }
    } finally { $env:PS2_VU_RECORD = $OldVu; $env:PS2_GPU_LOW_POWER = $OldGpu }
}
Write-Host "Native Windows executable built: $Runner"
Write-Host 'Not a gameplay certification. Check the build log and try the optional renderer.'
if ($Run) {
    if (-not $Disc -or -not (Test-Path -LiteralPath $Disc -PathType Leaf)) {
        Fail 'Provide your owned ISO with -Disc to run the compiled native game.'
    }
    RunProcess $Runner @($Elf,'--disc',(Resolve-Path -LiteralPath $Disc).Path,'--window','--trace-syscalls')
}