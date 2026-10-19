# Build host-only asset tools, extract owned game assets via external GPLv3
# Saru3VfiTool, then inventory stage scripts. No console machine emulation.
[CmdletBinding()]
param(
    [string]$Game = '',
    [string]$Saru = '',
    [string]$Out = '',
    [switch]$Reuse,
    [switch]$KeepTim2,
    [switch]$Gltf,
    [switch]$Animate,
    [string]$Ae3Cli = 'ae3'
)
$ErrorActionPreference = 'Stop'
if ($Animate) { $Gltf = $true }
$Project = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Native = Join-Path $Project 'native'
$Private = Join-Path $Native 'private'
if (-not $Out) { $Out = Join-Path $Private 'owned_assets' }

function Select-OwnerFile([string]$Title, [string]$Filter) {
    Add-Type -AssemblyName System.Windows.Forms
    $dialog = New-Object System.Windows.Forms.OpenFileDialog
    $dialog.Title = $Title
    $dialog.Filter = $Filter
    $dialog.CheckFileExists = $true
    if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
        throw "No input selected: $Title"
    }
    return $dialog.FileName
}
if (-not $Game) {
    $Game = Select-OwnerFile 'Choose your own Ape Escape 3 ISO or DATA.BIN' 'Ape Escape 3 ISO or DATA.BIN (*.iso;*.bin)|*.iso;*.bin|All files (*.*)|*.*'
}
if (-not $Saru) {
    $Saru = Select-OwnerFile 'Choose Saru3VfiTool.exe or its built DLL' 'Saru3VfiTool (*.exe;*.dll)|*.exe;*.dll|All files (*.*)|*.*'
}
if (-not (Test-Path -LiteralPath $Game -PathType Leaf)) {
    throw 'Original owner game file does not exist.'
}
if (-not (Test-Path -LiteralPath $Saru -PathType Leaf)) {
    throw 'Saru3VfiTool binary does not exist.'
}
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw 'Install Visual Studio C++ Desktop workload + CMake for the native Windows asset probe.'
}
if (-not (Get-Command py -ErrorAction SilentlyContinue)) {
    throw 'Python 3 Windows launcher (py) is needed for the offline preparation helper.'
}
$BuildDir = Join-Path $Native 'build'
$Probe = Join-Path $BuildDir 'Release\Getchu3AssetProbe.exe'
$StageProbe = Join-Path $BuildDir 'Release\Getchu3StageProbe.exe'
if (-not (Test-Path -LiteralPath $Probe) -or -not (Test-Path -LiteralPath $StageProbe)) {
    & cmake -S $Native -B $BuildDir -G 'Visual Studio 17 2022' -A x64
    if ($LASTEXITCODE -ne 0) { throw 'Native-only CMake configuration failed.' }
}
& cmake --build $BuildDir --config Release --target Getchu3AssetProbe Getchu3StageProbe --parallel
if ($LASTEXITCODE -ne 0) { throw 'Native host asset-tools build failed.' }

$Prep = Join-Path $Native 'tools\prepare_owned_assets.py'
$PrepArgs = @('-3', $Prep, '--game', $Game, '--out', $Out, '--saru', $Saru, '--probe', $Probe)
if ($Reuse) { $PrepArgs += '--reuse' }
if ($KeepTim2) { $PrepArgs += '--keep-tim2' }
& py @PrepArgs
if ($LASTEXITCODE -ne 0) {
    throw "Offline asset extraction failed. Your original game is unchanged; inspect $Out."
}

$ExtractedRoot = Join-Path $Out 'extracted'
$Inventory = Join-Path $Out 'native_stage_script_inventory.txt'
& $StageProbe $ExtractedRoot > $Inventory
if ($LASTEXITCODE -ne 0) {
    throw 'Native stage scan failed after extraction.'
}
if ($Gltf) {
    $Converter = Join-Path $Native 'tools\convert_models_to_glb.py'
    $GlbDir = Join-Path $Out 'glb'
    $modelArgs = @('-3', $Converter, '--extracted', $ExtractedRoot, '--out', $GlbDir, '--ae3', $Ae3Cli)
    if ($Animate) { $modelArgs += '--animations' }
    & py @modelArgs
    if ($LASTEXITCODE -ne 0) {
        throw 'Optional MIT ae3-sdk I3D-to-GLB model conversion failed.'
    }
    Write-Host "Original I3D model GLBs: $GlbDir"
}
Write-Host ''
Write-Host 'Getchu 3: NATIVE Windows asset preparation completed.'
Write-Host "Private extracted data: $ExtractedRoot"
Write-Host "Lua stage inventory: $Inventory"
Write-Host "Private summary: $(Join-Path $Out 'native_asset_inventory.json')"
Write-Host 'Original disc files were not modified. No PS2 CPU/GPU runtime was used.'
Write-Host 'This is preparation toward a native game, NOT playable gameplay.'
