# Native game-free test of real runtime code-window capture and AOT retranslation.
# Round 1 writes executable instructions into guest RAM then JALRs into the
# missing address; the dumped RAM image is captured and the missing PC rewritten.
# Round 2 compiles that specific image and proves execution through guest Exit.
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Upstream,
    [Parameter(Mandatory=$true)][string]$Compiler,
    [Parameter(Mandatory=$true)][string]$RuntimeBuild,
    [string]$VcpkgToolchain = '',
    [ValidateRange(10,180)][int]$TimeoutSeconds = 45
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Workspace = Join-Path $RuntimeBuild 'ae3-aot-overlay'
$Elf = Join-Path $Workspace 'synthetic-unpacker.elf'
$Generated = Join-Path $Workspace 'generated'
$Overlays = Join-Path $Workspace 'overlays'
$Entries = Join-Path $Workspace 'entries.txt'
$Misses = Join-Path $Workspace 'misses.txt'
$Ram = Join-Path $Workspace 'dump.bin'
$Report = Join-Path $Workspace 'overlay-aot-result.json'
$Runner = Join-Path $RuntimeBuild 'Release\ps2run.exe'
New-Item -ItemType Directory -Path $Workspace,$Overlays -Force | Out-Null
Set-Content -LiteralPath $Entries -Value '' -Encoding Ascii
function Checked([string]$App, [string[]]$Arg) {
    & $App @Arg
    if ($LASTEXITCODE -ne 0) { throw "Failed ($LASTEXITCODE): $App" }
}
function Translate() {
    Checked $Compiler @($Elf,'--entries',$Entries,'--overlays',$Overlays,'--out',$Generated)
    $cmakeArgs = @('-S',$Upstream,'-B',$RuntimeBuild,'-G','Visual Studio 17 2022',
                   '-A','x64','-T','ClangCL',"-DPS2_GENERATED=$Generated",'-DAE3_REQUIRE_SDL2=ON')
    if ($VcpkgToolchain) { $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchain" }
    Checked 'cmake' $cmakeArgs
    Checked 'cmake' @('--build',$RuntimeBuild,'--config','Release','--target','ps2run','--parallel')
    if (-not (Test-Path -LiteralPath $Runner)) { throw 'Native overlay runner not built' }
}
function Execute([string]$Stage,[bool]$ObserveRAM) {
    $err = Join-Path $Workspace ($Stage + '.stderr.txt')
    $out = Join-Path $Workspace ($Stage + '.stdout.txt')
    Remove-Item -LiteralPath $err,$out -ErrorAction SilentlyContinue
    $prevPeek = $env:PS2_PEEK
    $prevDump = $env:PS2_DUMP
    try {
        if ($ObserveRAM) { $env:PS2_PEEK = '00012000,4' }
        else { Remove-Item Env:PS2_PEEK -ErrorAction SilentlyContinue }
        $env:PS2_DUMP = $Ram
        $arg = @(('"' + $Elf + '"'),'--frames','120','--misses',('"' + $Misses + '"'))
        $proc = Start-Process -FilePath $Runner -ArgumentList $arg -PassThru -NoNewWindow -RedirectStandardError $err -RedirectStandardOutput $out
        if (-not $proc.WaitForExit($TimeoutSeconds*1000)) {
            Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
            throw "Overlay $Stage timed out"
        }
        return [pscustomobject]@{ code = $proc.ExitCode; trace = $err }
    } finally {
        $env:PS2_PEEK = $prevPeek
        $env:PS2_DUMP = $prevDump
    }
}
Checked 'py' @('-3',(Join-Path $PSScriptRoot 'synthetic_overlay_smoke.py'),'--out',$Elf)
Checked 'py' @('-3',(Join-Path $PSScriptRoot 'synthetic_overlay_smoke.py'),'--out',$Elf,'--check')
Translate
Remove-Item -LiteralPath $Misses,$Ram -ErrorAction SilentlyContinue
$first = Execute 'first-run' $false
if (-not (Test-Path -LiteralPath $Misses)) { throw "Initial JALR code miss not recorded; see $($first.trace)" }
if (-not (Test-Path -LiteralPath $Ram)) { throw "No guest RAM image after missing code; see $($first.trace)" }
$firstMiss = Get-Content -LiteralPath $Misses -Raw
if ($firstMiss -notmatch '(?i)\b00110000\b') { throw "Expected JALR into 00110000, misses were: $firstMiss" }
Checked 'py' @('-3',(Join-Path $Upstream 'scripts\windows.py'),'capture',$Elf,$Ram,$Overlays,$Misses)
$images = @(Get-ChildItem -LiteralPath $Overlays -Filter '*.bin' -File)
if ($images.Count -ne 1) { throw "Expected exactly one captured window image, found $($images.Count)" }
$overlayMiss = Get-Content -LiteralPath $Misses -Raw
if ($overlayMiss -notmatch '(?i)00110000_[0-9a-f]{8}:0\b') { throw "Missing PC not rebased into captured overlay: $overlayMiss" }
Copy-Item -LiteralPath $Misses -Destination $Entries -Force
Translate
Remove-Item -LiteralPath $Misses,$Ram -ErrorAction SilentlyContinue
$second = Execute 'recompiled-run' $true
$trace = Get-Content -LiteralPath $second.trace -Raw
if ($second.code -ne 0 -or $trace -notmatch '(?m)^\[peek\] 00012000:\s+0000008f\b' -or
    $trace -notmatch '(?m)^\[ee\] stopped after .*:\s*Exit\s*$') {
    throw "Recompiled code-window execution failed (exit $($second.code)), inspect $($second.trace)"
}
[ordered]@{
    result = 'PASS'
    original_game_code_used = $false
    observed_initial_code_miss = '00110000'
    discovered_executable_window = $images[0].Name
    dynamic_overlay_entry = $overlayMiss.Trim()
    recompiled_guest_store_address = '00012000'
    recompiled_guest_store_value = '0000008f'
    runner_exit_code = $second.code
} | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath $Report -Encoding UTF8
Write-Host "SYNTHETIC NATIVE CODE WINDOW PASS: missing JALR -> RAM capture -> overlay AOT -> 143 -> Exit"
Write-Host "Report: $Report"
