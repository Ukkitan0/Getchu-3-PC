# Interactive Windows entrypoint for a LOCAL user-owned ISO and native PS2 AOT.
# The game remains experimental. Never downloads or shares commercial data.
[CmdletBinding()]
param(
    [string]$Disc = '',
    [ValidateRange(0,64)][int]$ExploreRounds = 8,
    [switch]$SoftwareRenderer,
    [switch]$RunOnly
)
$ErrorActionPreference = 'Stop'
$Project = $PSScriptRoot
if (-not $Disc) {
    Add-Type -AssemblyName System.Windows.Forms
    $Dialog = New-Object System.Windows.Forms.OpenFileDialog
    $Dialog.Title = 'Select your own Ape Escape 3 (USA) ISO image'
    $Dialog.Filter = 'PS2 disc images (*.iso)|*.iso|All files (*.*)|*.*'
    $Dialog.CheckFileExists = $true
    if ($Dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
        throw 'Disc selection cancelled; no build or game data was changed.'
    }
    $Disc = $Dialog.FileName
}
if (-not (Test-Path -LiteralPath $Disc -PathType Leaf)) {
    throw "Selected ISO does not exist: $Disc"
}
$Disc = (Resolve-Path -LiteralPath $Disc).Path
$Workspace = Join-Path $Project 'fastlane\private'
if ($RunOnly) {
    $NativeInfo = Join-Path $Workspace 'owned_disc\disc_prepare.json'
    $Runner = Join-Path $Workspace 'build-native-ae3\Release\ps2run.exe'
    if (-not (Test-Path -LiteralPath $NativeInfo) -or -not (Test-Path -LiteralPath $Runner)) {
        throw 'No previous owner-local native test executable. Launch without -RunOnly to build.'
    }
    $Prep = Get-Content -LiteralPath $NativeInfo -Raw | ConvertFrom-Json
    if ($Prep.disc -ne $Disc) { throw 'Selected ISO differs from built game. Rebuild without -RunOnly.' }
    if (-not (Test-Path -LiteralPath $Prep.executable)) {
        throw 'Previous private executable missing. Rebuild without -RunOnly.'
    }
    $Card = Join-Path $Workspace 'ae3_native_save.ps2'
    & $Runner $Prep.executable '--disc' $Disc '--window' '--memcard' $Card '--trace-syscalls'
    if ($LASTEXITCODE -ne 0) { throw "Native test exited with code $LASTEXITCODE" }
} else {
    $Args = @('-Disc',$Disc,'-ExploreRounds',$ExploreRounds,'-ExploreFrames',1800,'-Run')
    if ($SoftwareRenderer) { $Args += '-SoftwareRenderer' }
    & (Join-Path $Project 'RUN_BULK_NATIVE_WINDOWS.ps1') @Args
    if (-not $?) { throw 'Native build or test failed; inspect fastlane/private/build.log.' }
}
