# Game-free end-to-end Windows PowerShell invocation contract.
# Uses a fake private ISO and a replacement build script with the SAME
# validated SeedMode and ExploreRounds parameter signatures as production.
# This catches the bug where array splatting passed 8 as a positional SeedMode.
$ErrorActionPreference = 'Stop'
$Original = Join-Path (Split-Path -Parent $PSScriptRoot) 'TEST_APE_ESCAPE_3_WINDOWS.ps1'
if (-not (Test-Path -LiteralPath $Original)) { throw "Missing launcher $Original" }

$Folder = Join-Path $env:RUNNER_TEMP ("ae3-argument-binding-" + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $Folder -Force | Out-Null
try {
    Copy-Item -LiteralPath $Original -Destination (Join-Path $Folder 'TEST_APE_ESCAPE_3_WINDOWS.ps1')
    $Disc = Join-Path $Folder 'synthetic disc with spaces.iso'
    [System.IO.File]::WriteAllText($Disc, 'fixture file not containing copyrighted game data')
    $Result = Join-Path $Folder 'call.json'
    $FakeBuild = Join-Path $Folder 'RUN_BULK_NATIVE_WINDOWS.ps1'
    # This defines the same positional order and ValidateSet as the real script.
    @'
[CmdletBinding()]
param(
    [string]$Elf = "", [string]$Disc = "", [string]$Workspace = "",
    [switch]$Run,
    [ValidateSet('constructors','common','all')][string]$SeedMode = 'constructors',
    [switch]$ReuseUpstream,
    [ValidateRange(0,64)][int]$ExploreRounds = 0,
    [ValidateRange(60,20000)][int]$ExploreFrames = 1800,
    [ValidateRange(30,1800)][int]$ExploreTimeoutSeconds = 300,
    [string]$WgpuDir = "",
    [string]$VcpkgRoot = "",
    [switch]$SoftwareRenderer
)
[ordered]@{
    Disc = $Disc
    SeedMode = $SeedMode
    ExploreRounds = $ExploreRounds
    ExploreFrames = $ExploreFrames
    Run = [bool]$Run
    SoftwareRenderer = [bool]$SoftwareRenderer
} | ConvertTo-Json | Set-Content -LiteralPath $env:AE3_TEST_RESULT -Encoding UTF8
'@ | Set-Content -LiteralPath $FakeBuild -Encoding UTF8

    $env:AE3_TEST_RESULT = $Result
    $Launcher = Join-Path $Folder 'TEST_APE_ESCAPE_3_WINDOWS.ps1'
    foreach ($Scenario in @(
        @{ Rounds = 8; Soft = $false },
        @{ Rounds = 12; Soft = $true },
        @{ Rounds = 0; Soft = $false }
    )) {
        Remove-Item -LiteralPath $Result -ErrorAction SilentlyContinue
        $Arguments = @{ Disc = $Disc; ExploreRounds = $Scenario.Rounds }
        if ($Scenario.Soft) { $Arguments['SoftwareRenderer'] = $true }
        & $Launcher @Arguments
        if (-not (Test-Path -LiteralPath $Result)) {
            throw "Production launcher failed to invoke mock build for round count $($Scenario.Rounds)"
        }
        $Actual = Get-Content -LiteralPath $Result -Raw | ConvertFrom-Json
        if ($Actual.Disc -ne $Disc -or $Actual.SeedMode -ne 'constructors' -or
            $Actual.ExploreRounds -ne $Scenario.Rounds -or
            $Actual.ExploreFrames -ne 1800 -or -not $Actual.Run -or
            $Actual.SoftwareRenderer -ne $Scenario.Soft) {
            throw "Incorrect named parameter binding for $($Scenario.Rounds) rounds. Actual: $(Get-Content -LiteralPath $Result -Raw)"
        }
    }
    Write-Host 'AE3_LAUNCHER_ARGUMENT_BINDING_OK: 8/12/0 rounds, original SeedMode preserved, software mode passed'
} finally {
    Remove-Item Env:AE3_TEST_RESULT -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $Folder -Recurse -Force -ErrorAction SilentlyContinue
}
