@echo off
setlocal
rem Native Windows C/C++ build only. No guest interpreter, fake playable scene,
rem original retail binary, or GitHub Actions run is bundled.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0BUILD_NATIVE_WINDOWS.ps1" %*
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" echo Native C++ build, test, or authentic retail boot blocked. Exit %RC%.
exit /b %RC%
