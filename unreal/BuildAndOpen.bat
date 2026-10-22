@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\build_and_open.ps1" %*
if errorlevel 1 (
    echo.
    echo ERROR: Unreal project or authentic native Lua bridge build failed.
    echo No playable release was produced. Review the error above.
    pause
    exit /b 1
)
exit /b 0
