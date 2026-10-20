@echo off
setlocal
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0RECOVER_ORIGINAL_GAME_WINDOWS.ps1" %*
if errorlevel 1 (
  echo.
  echo Original ISO extraction or audit failed. See the error above.
  pause
  exit /b 1
)
echo.
echo Original Ape Escape 3 assets and Lua audit are available under native\private.
pause
