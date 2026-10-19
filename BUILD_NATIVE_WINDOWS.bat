@echo off
setlocal EnableExtensions
cd /d "%~dp0"
echo.
echo ==================================================================
echo  GETCHU 3 - NATIVE WINDOWS BUILD (NO PS2 HARDWARE EMULATION)
echo ==================================================================
echo.
where cmake >nul 2>nul
if errorlevel 1 (
    echo CMake missing. Install Visual Studio 2022 with C++ Desktop workload and CMake.
    pause
    exit /b 1
)
cmake -S "native" -B "native\build" -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto failed
cmake --build "native\build" --config Release --parallel
if errorlevel 1 goto failed
ctest --test-dir "native\build" -C Release --output-on-failure
if errorlevel 1 goto failed
if not exist "native\build\Release\Getchu3Native.exe" goto failed
echo.
echo Native engine and real game archive reader built successfully.
echo The game itself is NOT playable yet: this is a real Windows engine and asset test.
echo No ps2-web-recomp, PCSX2, console CPU, IOP, or graphics emulation is used.
echo.
start "" "native\build\Release\Getchu3Native.exe" %*
exit /b 0
:failed
echo.
echo NATIVE WINDOWS BUILD FAILED. The previous PS2 runtime will NOT be started.
pause
exit /b 1
