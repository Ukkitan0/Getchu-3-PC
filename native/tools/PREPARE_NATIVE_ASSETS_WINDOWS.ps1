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
    [string]$Unluac = '',
    [switch]$AllLua,
    [switch]$StrictOriginalStage,
    [string]$LaunchOriginalStage = '',
    [string]$OriginalSpawn = 'spawn_default',
    [string]$Ae3Cli = 'ae3'
)
$ErrorActionPreference = 'Stop'
if ($Animate) { $Gltf = $true }
if ($LaunchOriginalStage) { $Gltf = $true; $Animate = $true }
# Actual retail Lua runs directly from the ISO; unluac/recovered .lua is NOT a prerequisite.
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
# Original level geometry and textures often live within retail bg.pck,
# not as loose .i3d/.tm2 files. Unpack only real owner-local model,
# animation and texture members before the existing GLB converter runs.
# Collision is decoded from PCK in memory by the native C++ engine.
$PckUnpacker = Join-Path $Native 'tools\unpack_original_stage_pck.py'
$PckArgs = @('-3', $PckUnpacker, '--extracted', $ExtractedRoot)
if ($LaunchOriginalStage) { $PckArgs += @('--stage', $LaunchOriginalStage) }
& py @PckArgs
if ($LASTEXITCODE -ne 0) {
    throw 'Original game PCK unpack failed; no replacement models or fake level will be made.'
}
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
# Original retail game scripts are required for authentic gameplay.
# Never silently substitute a hand-written test area.lua in the actual path.
$LuaOut = Join-Path $Out 'recovered_lua'
if ($Unluac) {
    if (-not (Test-Path -LiteralPath $Unluac -PathType Leaf)) {
        throw "Lua 5.0 unluac JAR missing: $Unluac"
    }
    if (-not (Get-Command java -ErrorAction SilentlyContinue)) {
        throw "Java is required for OFFLINE recovery of the original game's Lua 5.0 scripts."
    }
    $argsLua = @('-3', (Join-Path $Native 'tools\recover_lua50.py'),
        '--extracted', $ExtractedRoot, '--out', $LuaOut,
        '--unluac', $Unluac)
    if ($AllLua) { $argsLua += '--all-scripts' }
    & py @argsLua
    if ($LASTEXITCODE -ne 0) {
        throw 'Recovery of original game Lua 5.0 source failed; no synthetic replacement will be created.'
    }
}
$StageAuditArgs = @('-3', (Join-Path $Native 'tools\audit_original_stage.py'),
    '--extracted', $ExtractedRoot,
    '--out', (Join-Path $Out 'original_stage_audit.json'))
if ($Gltf) { $StageAuditArgs += @('--glb', (Join-Path $Out 'glb')) }
if ($Unluac) { $StageAuditArgs += @('--lua', $LuaOut) }
if ($StrictOriginalStage) { $StageAuditArgs += '--require-real-stage' }
& py @StageAuditArgs
if ($LASTEXITCODE -ne 0) {
    throw 'Original Ape Escape 3 stage is incomplete. Read original_stage_audit.json for the specific missing original assets.'
}
if ($LaunchOriginalStage) {
    # Launch ONLY with original retail stage data. No mock level, no
    # hand-authored substitute scripts, and no PlayStation runtime.
    $NativeExe = Join-Path $BuildDir 'Release\Getchu3Native.exe'
    $RetailAudit = Join-Path $BuildDir 'Release\Getchu3RetailStageAudit.exe'
    & cmake --build $BuildDir --config Release --target Getchu3Native Getchu3RetailStageAudit --parallel
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $NativeExe -PathType Leaf)) {
        throw 'Native original-game Windows C++ renderer did not build.'
    }
    # Execute the REAL original game script through native Lua 5.0 before
    # attempting scene startup. Any missing native game binding is a hard
    # blocker, not a pretend successful original level boot.
    & $RetailAudit $Game $LaunchOriginalStage $OriginalSpawn
    if ($LASTEXITCODE -ne 0) {
        throw 'Original retail area.luc did not execute successfully in native Lua 5.0. Restore its missing game-engine bindings before launching the original level.'
    }
    $LaunchTool = Join-Path $Native 'tools\launch_owned_retail_stage.py'
    $LaunchArgs = @('-3', $LaunchTool, '--iso', $Game,
        '--stage', $LaunchOriginalStage, '--extracted', $ExtractedRoot,
        '--glb', (Join-Path $Out 'glb'), '--exe', $NativeExe,
        '--spawn', $OriginalSpawn,
        '--report', (Join-Path $Out 'original_retail_stage_launch.json'))
    & py @LaunchArgs
    if ($LASTEXITCODE -ne 0) {
        throw 'Authentic Ape Escape 3 stage launch failed; see original_retail_stage_launch.json. No synthetic fallback was executed.'
    }
}
Write-Host ''
Write-Host 'Getchu 3: NATIVE Windows asset preparation completed.'
Write-Host "Private extracted data: $ExtractedRoot"
Write-Host "Lua stage inventory: $Inventory"
Write-Host "Original stage validation: $(Join-Path $Out 'original_stage_audit.json')"
Write-Host "Private summary: $(Join-Path $Out 'native_asset_inventory.json')"
Write-Host 'Original disc files were not modified. No PS2 CPU/GPU runtime was used.'
Write-Host 'This is preparation toward a native game, NOT playable gameplay.'
