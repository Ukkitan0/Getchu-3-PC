@echo off
setlocal
call "%~dp0BUILD_NATIVE_WINDOWS.bat" %*
exit /b %errorlevel%
