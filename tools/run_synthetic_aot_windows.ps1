# Game-free, end-to-end Windows test of the REAL upstream EE recompilation path:
# MIPS ELF -> eerecomp C++ -> ClangCL ps2run.exe -> guest JAL/JALR/syscall.
# This is not an Ape Escape 3 build and uses no retail files.
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Upstream,
    [Parameter(Mandatory=$true)][string]$Compiler,
    [Parameter(Mandatory=$true)][string]$RuntimeBuild,
    [string]$Workspace = '',
    [string]$VcpkgToolchain = '',
    [ValidateRange(5,180)][int]$TimeoutSeconds = 30
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
if (-not $Workspace) { $Workspace = Join-Path $RuntimeBuild 'ae3-aot-smoke' }
New-Item -ItemType Directory -Path $Workspace -Force | Out-Null
$elf = Join-Path $Workspace 'aot-original-test.elf'
$entries = Join-Path $Workspace 'dynamic-entries.txt'
$generated = Join-Path $Workspace 'generated'
$log = Join-Path $Workspace 'aot-smoke-stderr.txt'
$stdOut = Join-Path $Workspace 'aot-smoke-stdout.txt'
$report = Join-Path $Workspace 'aot-smoke-result.json'
function InvokeChecked([string]$Command, [string[]]$Arguments) {
    & $Command @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $($LASTEXITCODE): $Command"
    }
}
# Source ELF and callback entry list are synthesized from original test opcodes.
InvokeChecked 'py' @('-3', (Join-Path $PSScriptRoot 'synthetic_aot_smoke.py'), '--out', $elf, '--entries', $entries)
InvokeChecked 'py' @('-3', (Join-Path $PSScriptRoot 'synthetic_aot_smoke.py'), '--out', $elf, '--check')
InvokeChecked $Compiler @($elf, '--entries', $entries, '--out', $generated)
$cpp = @(Get-ChildItem -LiteralPath $generated -Filter '*.cpp' -File)
if ($cpp.Count -eq 0) { throw 'eerecomp emitted zero generated C++ translation units' }
$cmakeArgs = @('-S', $Upstream, '-B', $RuntimeBuild,
               '-G', 'Visual Studio 17 2022', '-A', 'x64', '-T', 'ClangCL',
               "-DPS2_GENERATED=$generated", '-DAE3_REQUIRE_SDL2=ON')
if ($VcpkgToolchain) { $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchain" }
InvokeChecked 'cmake' $cmakeArgs
InvokeChecked 'cmake' @('--build', $RuntimeBuild, '--config', 'Release', '--target', 'ps2run', '--parallel')
$runner = Join-Path $RuntimeBuild 'Release\ps2run.exe'
if (-not (Test-Path -LiteralPath $runner -PathType Leaf)) { throw "No native AOT runner at $runner" }
$oldPeek = $env:PS2_PEEK
$process = $null
try {
    $env:PS2_PEEK = '00012000,4'
    # Start-Process avoids PowerShell treating intentional game exit as a
    # pipeline exception, and allows a hard timeout on infinite guest loops.
    $process = Start-Process -FilePath $runner -ArgumentList ('"' + $elf + '"') -PassThru -NoNewWindow `
        -RedirectStandardError $log -RedirectStandardOutput $stdOut
    if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        throw "Synthetic AOT guest did not halt within $TimeoutSeconds seconds"
    }
    if ($process.ExitCode -ne 0) { throw "AOT ps2run exited with $($process.ExitCode)" }
} finally {
    if ($null -eq $oldPeek) { Remove-Item Env:PS2_PEEK -ErrorAction SilentlyContinue }
    else { $env:PS2_PEEK = $oldPeek }
}
$trace = Get-Content -LiteralPath $log -Raw
if ($trace -notmatch '(?m)^\[peek\] 00012000:\s+00001234\b') {
    throw "Guest direct JAL, indirect JALR, SW or RAM read failed; inspect $log"
}
if ($trace -notmatch '(?m)^\[ee\] stopped after .*:\s*Exit\s*$') {
    throw "Guest did not exit through actual PS2 kernel syscall 0x04; inspect $log"
}
$result = [ordered]@{
    result = 'PASS'
    fixture = 'original game-free R5900 JAL/JALR/memory/syscall code'
    original_commercial_code = $false
    eerecomp_cpp_units = $cpp.Count
    guest_entry = '00100100'
    indirect_entry_seed = '00100160'
    actual_guest_store_address = '00012000'
    actual_guest_store_value = '00001234'
    exit_syscall = '04'
    runner_exit_code = $process.ExitCode
    trace = $log
}
$result | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath $report -Encoding UTF8
Write-Host 'AE3 SYNTHETIC NATIVE AOT SMOKE PASS: real original JAL/JALR, memory store, kernel Exit.'
Write-Host "Report: $report"