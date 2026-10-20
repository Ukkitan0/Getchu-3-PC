@echo off
setlocal
set "PROJECT=%~dp0"
set "WORKSPACE=%PROJECT%fastlane\private"
where py >nul 2>nul || (echo Python 3 launcher "py" was not found & exit /b 1)
py -3 "%PROJECT%tools\diagnostic_bundle.py" --workspace "%WORKSPACE%" --out "%WORKSPACE%\AE3_NATIVE_DIAGNOSTICS.zip"
if errorlevel 1 (
    echo Unable to collect diagnostics; check fastlane\private\build.log
    pause
    exit /b 1
)
echo.
echo Share only this sanitized metadata ZIP when reporting a native boot failure:
echo "%WORKSPACE%\AE3_NATIVE_DIAGNOSTICS.zip"
pause
