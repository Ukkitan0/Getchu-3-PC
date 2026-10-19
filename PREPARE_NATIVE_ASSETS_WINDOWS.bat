@echo off
setlocal EnableExtensions
cd /d "%~dp0"
echo.
echo ===============================================================
echo   GETCHU 3 - ORIGINAL GAME ASSET PREPARATION (HOST NATIVE)
echo ===============================================================
echo.
echo Choose your owned AE3 disc and Saru3VfiTool binary in the file pickers.
echo Original game files remain private. No console emulator is started.
echo.
powershell.exe -NoProfile -STA -ExecutionPolicy Bypass -File "%~dp0native\tools\PREPARE_NATIVE_ASSETS_WINDOWS.ps1" %*
if errorlevel 1 (
  echo.
  echo Native-only asset preparation did not complete.
  pause
  exit /b 1
)
echo.
echo See native\private\owned_assets for extracted assets and script inventory.
pause
exit /b 0
