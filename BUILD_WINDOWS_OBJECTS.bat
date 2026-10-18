@echo off
setlocal EnableExtensions
cd /d "%~dp0"
echo [AE3] Verifying original retail executable and generating native C...
if not defined AE3_ELF set "AE3_ELF=%~dp0SCUS_975.01"
where py >nul 2>&1
if errorlevel 1 (
 echo Python launcher py.exe not found.
 exit /b 1
)
py -3 bulk_lift.py
if errorlevel 1 exit /b 1
py -3 cfg_lift.py
if errorlevel 1 exit /b 1
where clang-cl >nul 2>&1
if errorlevel 1 (
 echo clang-cl not found. Install LLVM or Visual Studio C++ Clang tools.
 exit /b 2
)
if not exist build mkdir build
clang-cl /nologo /std:c11 /O2 /c ae3_scalar_leaves.c /Fobuild\ae3_scalar_leaves.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /O2 /c ae3_control_leaves.c /Fobuild\ae3_control_leaves.obj
if errorlevel 1 exit /b 1
echo [AE3] Compiled native C into x64 object files (NOT a playable executable).
echo [AE3] build\ae3_scalar_leaves.obj
echo [AE3] build\ae3_control_leaves.obj
exit /b 0
