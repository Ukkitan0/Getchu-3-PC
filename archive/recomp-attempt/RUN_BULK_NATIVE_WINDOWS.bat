@echo off
setlocal
echo The PS2 hybrid recompilation pipeline is RETIRED for this project.
echo Starting the real native Windows C++ / Direct3D11 build instead.
call "%~dp0BUILD_NATIVE_WINDOWS.bat" %*
exit /b %errorlevel%
