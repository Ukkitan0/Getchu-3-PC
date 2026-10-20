@echo off
setlocal EnableExtensions
cd /d "%~dp0"
echo.
echo ===============================================================
echo  GETCHU 3 - NATIVE LUA + THREE 3D ACTORS TEST (NO EMULATION)
echo ===============================================================
echo.
if not exist "native\tools\make_actor_scene_fixture.py" (
    echo ERROR: Update this project from main first.
    pause
    exit /b 1
)
set "PYTHON_CMD=py -3"
where py >nul 2>nul
if errorlevel 1 set "PYTHON_CMD=python"
%PYTHON_CMD% "native\tools\make_actor_scene_fixture.py" "native\build\native_lua_actor_stage"
if errorlevel 1 (
    echo ERROR: Python 3 is needed for the legal test asset generator.
    pause
    exit /b 1
)
echo.
echo This demo has no copyrighted game data and contains:
echo - Native generated 3D platform/environment
echo - Native generated placeholder Player + two Monkey 3D models
echo - Original-style Lua 5.0 setupArea/new_ent scripts
echo - Separate native actor physics and animation slots
echo.
echo The result is a stage/actor proof of concept, NOT Ape Escape 3 gameplay.
echo.
call "BUILD_NATIVE_WINDOWS.bat" "%CD%\native\build\native_lua_actor_stage" "%CD%\native\build\native_lua_actor_stage\area.lua"
exit /b %errorlevel%
