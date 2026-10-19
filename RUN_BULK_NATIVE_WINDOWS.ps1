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
if (-not $Elf -or -not (Test-Path -LiteralPath $Elf -PathType Leaf)) {
    Fail 'Retail US SCUS_975.01 ELF not found: supply -Elf "C:\path\SCUS_975.01".'
}
$Elf = (Resolve-Path -LiteralPath $Elf).Path
$ActualHash = (Get-FileHash -LiteralPath $Elf -Algorithm SHA256).Hash
if ($ActualHash -ne $ExpectedHash) { Fail "Wrong executable: $ActualHash, expected $ExpectedHash" }
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
# The overlay is idempotent and anchored to reviewed upstream exact source.
$Report = Join-Path $Workspace 'windows_port_report.json'
RunProcess 'py' @('-3',(Join-Path $Project 'tools\windows_upstream_patch.py'),'--upstream',$Upstream,'--report',$Report)
if (-not $VcpkgRoot) { $VcpkgRoot = $env:VCPKG_ROOT }
$Toolchain = ''
if ($VcpkgRoot) {
    $Candidate = Join-Path $VcpkgRoot 'scripts\buildsystems\vcpkg.cmake'
    if (Test-Path -LiteralPath $Candidate) { $Toolchain = $Candidate }
}
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
RunProcess 'py' @('-3',(Join-Path $Project 'tools\bulk_recompile.py'),'--elf',$Elf,'--out',$Entries,'--seed-mode',$SeedMode,'--eerecomp',$Compiler,'--cpp-out',$Generated)
$EntryList = Join-Path $Entries 'entries.txt'
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
            RunProcess 'py' @('-3',(Join-Path $Project 'tools\native_explore.py'),'--entries',$EntryList,'--misses',$Misses,'--inventory',(Join-Path $Project 'analysis\full_elf\ee_text_word_inventory.tsv.gz'),'--report',(Join-Path $Workspace 'explore_last.json'))
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