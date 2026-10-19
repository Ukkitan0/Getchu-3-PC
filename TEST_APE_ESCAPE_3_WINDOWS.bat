@echo off
setlocal
set "PROJECT=%~dp0"
where powershell >nul 2>nul || (echo Windows PowerShell is required.& pause & exit /b 1)
powershell -NoProfile -STA -ExecutionPolicy Bypass -File "%PROJECT%TEST_APE_ESCAPE_3_WINDOWS.ps1" %*
set "RESULT=%ERRORLEVEL%"
if not "%RESULT%"=="0" (
    echo.
    echo Native experiment failed; see fastlane\private\build.log and boot_report.json
    pause
)
exit /b %RESULT%
