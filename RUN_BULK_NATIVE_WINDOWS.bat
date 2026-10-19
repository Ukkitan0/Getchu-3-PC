@echo off
setlocal
set "PROJECT=%~dp0"
where powershell >nul 2>nul || (echo PowerShell missing & exit /b 1)
powershell -NoProfile -ExecutionPolicy Bypass -File "%PROJECT%RUN_BULK_NATIVE_WINDOWS.ps1" %*
set "RESULT=%ERRORLEVEL%"
if not "%RESULT%"=="0" echo See fastlane\private\build.log for compiler diagnostics.
exit /b %RESULT%