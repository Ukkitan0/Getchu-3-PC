@echo off
setlocal EnableExtensions
cd /d "%~dp0"
if not defined AE3_ELF set "AE3_ELF=%~dp0SCUS_975.01"
if not exist "%AE3_ELF%" (
  echo [AE3] Please set AE3_ELF to your locally owned original SCUS_975.01 file.
  exit /b 1
)
where clang-cl >nul 2>&1
if errorlevel 1 (
  echo [AE3] clang-cl is required. Use a VS2022 Developer Command Prompt with C++ Clang tools.
  exit /b 2
)
if not exist build mkdir build
clang-cl /nologo /std:c11 /W4 /WX /O2 /Fe:build\AE3BootProbe.exe ^
  ae3_elf_image.c ae3_boot_prefix.c ae3_boot_memory_fill.c ^
  ae3_boot_thread_setup.c ae3_boot_kernel_bridge.c ae3_boot_init_prefix.c ^
  ae3_boot_semaphore_init.c ae3_ee_sema_kernel.c ae3_boot_hook_init.c ^
  ae3_boot_probe.c
if errorlevel 1 exit /b 3
echo [AE3] Built a boot diagnostic ONLY, not a playable game.
if "%AE3_BOOT_TEST_HLE%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-kernel-fixture --test-sema-hle
) else (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%"
)
exit /b %errorlevel%