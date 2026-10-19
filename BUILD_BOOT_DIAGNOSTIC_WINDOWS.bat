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
  ae3_boot_semaphore_init.c ae3_ee_sema_kernel.c ae3_ee_syscall_kernel.c ae3_boot_hook_init.c ^
  ae3_ee_memory_bus.c ae3_guest_find_address.c ae3_boot_post_find.c ae3_boot_next_init.c ae3_ee_timer3.c ae3_boot_timer3_resume.c ae3_guest_kernel_copy.c ae3_boot_timer3_syscalls.c ae3_ee_boot_services.c ae3_guest_get_entry.c ae3_boot_post_patch.c ae3_boot_alloc_init.c ae3_ee_intc_kernel.c ae3_boot_post_intc.c ae3_boot_intc_critical.c ae3_ee_timer2.c ae3_boot_timer2_continue.c ae3_boot_timer2_setup.c ae3_boot_enable_intc.c ae3_boot_timer2_recheck.c ae3_boot_probe.c
if errorlevel 1 exit /b 3
echo [AE3] Built a boot diagnostic ONLY, not a playable game.
if "%AE3_BOOT_TEST_TIMER2%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer2-mode
) else if "%AE3_BOOT_TEST_ALLOC_INIT%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer3-copy-alloc
) else if "%AE3_BOOT_TEST_TIMER3_COPY%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer3-copy
) else if "%AE3_BOOT_TEST_TIMER3%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer3
) else if "%AE3_BOOT_NEXT_MMIO%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-next-mmio
) else if "%AE3_BOOT_POST_FIND%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-post-find
) else if "%AE3_BOOT_MODEL%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-ee-boot-model --test-sema-hle --test-ee-alias
) else if "%AE3_BOOT_TEST_ALIAS%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-kernel-fixture --test-sema-hle --test-ee-alias
) else if "%AE3_BOOT_TEST_HLE%"=="1" (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%" --test-kernel-fixture --test-sema-hle
) else (
  "%~dp0build\AE3BootProbe.exe" "%AE3_ELF%"
)
exit /b %errorlevel%
