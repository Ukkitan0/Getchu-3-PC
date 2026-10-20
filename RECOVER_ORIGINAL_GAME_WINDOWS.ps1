# Recover the original Ape Escape 3 (USA) game from the owner's own ISO.
# Original Lua files, complete VFI assets and SCUS_975.01 stay PRIVATE.
# No PS2 emulation, synthetic levels, or modified game-disc content.
[CmdletBinding()]
param(
    [string]$ISO = '',
    [string]$Output = '',
    [switch]$OnlyLua,
    [string]$Unluac = ''
)
$ErrorActionPreference = 'Stop'
$Root = $PSScriptRoot
$Tools = Join-Path $Root 'native\tools'
if (-not (Get-Command py -ErrorAction SilentlyContinue)) {
    throw 'Python 3 (py launcher) is required to read the original ISO.'
}
if (-not $ISO) {
    $PriorGame = 'C:\ApeEscape3\Ape Escape 3 (USA).iso'
    if (Test-Path -LiteralPath $PriorGame -PathType Leaf) {
        $ISO = $PriorGame
    } else {
        Add-Type -AssemblyName System.Windows.Forms
        $dialog = New-Object System.Windows.Forms.OpenFileDialog
        $dialog.Title = 'Choose the original Ape Escape 3 USA ISO'
        $dialog.Filter = 'Original ISO files (*.iso)|*.iso'
        $dialog.CheckFileExists = $true
        if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
            throw 'No original ISO selected. Nothing was extracted.'
        }
        $ISO = $dialog.FileName
    }
}
if (-not (Test-Path -LiteralPath $ISO -PathType Leaf)) {
    throw "Original ISO not found: $ISO"
}
$ISO = (Resolve-Path -LiteralPath $ISO).Path
if (-not $Output) {
    $Output = Join-Path $Root (
        'native\private\original_iso_' + (Get-Date -Format 'yyyyMMdd_HHmmss_fff'))
}
$Output = [System.IO.Path]::GetFullPath($Output)
if (Test-Path -LiteralPath $Output) {
    if (@(Get-ChildItem -LiteralPath $Output -Force).Count -ne 0) {
        throw "Extraction destination must be EMPTY: $Output"
    }
}
Write-Host "Original Ape Escape 3 ISO: $ISO"
Write-Host "PRIVATE output directory: $Output"
Write-Host 'Recovering unmodified original retail Lua 5.0 bytecode and SCUS_975.01...'
$Extract = Join-Path $Tools 'extract_retail_iso.py'
$ExtractArgs = @('-3',$Extract,$ISO,'--out',$Output)
if (-not $OnlyLua) { $ExtractArgs += '--all' }
& py @ExtractArgs
if ($LASTEXITCODE -ne 0) { throw 'Original ISO extraction failed. No replacement data was generated.' }

$LuaDir = Join-Path $Output 'vfi'
$LuaAudit = Join-Path $Output 'original_lua_engine_api_audit.json'
& py -3 (Join-Path $Tools 'analyze_original_lua50.py') $LuaDir '--all-scripts' '--out' $LuaAudit
if ($LASTEXITCODE -ne 0) {
    throw 'Original Lua opcode audit failed; preserve the original .luc files for investigation.'
}

$Elf = Join-Path $Output 'SCUS_975.01'
$RegistrationMap = Join-Path $Output 'original_elf_lua_registrations.json'
& py -3 (Join-Path $Tools 'map_original_lua_registrations.py') $Elf '--out' $RegistrationMap
if ($LASTEXITCODE -ne 0) {
    Write-Warning 'Original executable does not match the previously mapped USA revision. Keep the original ELF; no callback names will be invented.'
}

if (-not $OnlyLua) {
    # Original PCK models/textures/animations are extracted into the same
    # private stage tree for later native C++/D3D11 conversion. Collision
    # remains in PCK and is decoded directly by Getchu3Native.
    & py -3 (Join-Path $Tools 'unpack_original_stage_pck.py') '--extracted' $LuaDir
    if ($LASTEXITCODE -ne 0) {
        throw 'Native original PCK asset unpack failed. Check the actual original member table.'
    }
}
if ($Unluac) {
    if (-not (Test-Path -LiteralPath $Unluac -PathType Leaf)) {
        throw "Provided Lua 5.0 decompiler does not exist: $Unluac"
    }
    if (-not (Get-Command java -ErrorAction SilentlyContinue)) {
        throw 'Java is required ONLY for optional offline Lua source decompilation.'
    }
    $Recovered = Join-Path $Output 'decompiled_lua_private'
    & py -3 (Join-Path $Tools 'recover_lua50.py') '--extracted' $LuaDir '--out' $Recovered '--unluac' $Unluac '--all-scripts'
    if ($LASTEXITCODE -ne 0) {
        Write-Warning 'Optional Lua source decompilation was incomplete. Original .luc bytecode and native API audit remain available.'
    }
}
# Small owner-private uploadable package: ALL original Lua bytecode,
# original executable and analysis. The full ISO and unrelated assets remain
# in the private extraction directory on the owner's Windows machine.
$PrivateHandoff = Join-Path $Output 'AE3_ORIGINAL_LUA_AND_ELF_PRIVATE.zip'
& py -3 (Join-Path $Tools 'package_private_retail_evidence.py') '--source' $Output '--out' $PrivateHandoff
if ($LASTEXITCODE -ne 0) {
    throw 'Original private Lua+ELF handoff packaging failed; extracted original files remain intact.'
}
Write-Host ''
Write-Host 'Original retail game extraction and native API analysis completed.'
Write-Host "ALL original Lua 5.0 bytecode: $LuaDir"
Write-Host "Original native-engine Lua API report: $LuaAudit"
Write-Host "Original ELF Lua registration sites: $RegistrationMap"
Write-Host "Original ISO manifest: $(Join-Path $Output 'original_iso_recovery_manifest.json')"
Write-Host "Owner-private Lua and original ELF handoff: $PrivateHandoff"
Write-Host 'The original ELF and game resources are NOT committed or published.'
Write-Host 'Extracted files are original retail data; they are NOT a playable Windows port.'
