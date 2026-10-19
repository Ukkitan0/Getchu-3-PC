# Kept only as a backwards-compatible launch command.
# The former PS2 runtime is permanently removed from the active workflow.
[CmdletBinding()]
param([string]$Disc = '')
$native = Join-Path $PSScriptRoot 'BUILD_NATIVE_WINDOWS.bat'
if (-not (Test-Path -LiteralPath $native)) {
    throw 'Native-only build script missing. Update to the latest Getchu-3-PC checkout.'
}
if ($Disc) { & $native $Disc } else { & $native }
if ($LASTEXITCODE -ne 0) { throw "Native Windows build failed with $LASTEXITCODE" }
