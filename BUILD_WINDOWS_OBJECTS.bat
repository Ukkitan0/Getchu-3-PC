@echo off
setlocal EnableExtensions
cd /d "%~dp0"
echo [AE3] Verifying original executable and generating native C...
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
py -3 loop_lift.py
if errorlevel 1 exit /b 1
py -3 resume_lift.py
if errorlevel 1 exit /b 1
py -3 call_lift.py
if errorlevel 1 exit /b 1
py -3 stack_lift.py
if errorlevel 1 exit /b 1
py -3 audit_boot_entry.py
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
clang-cl /nologo /std:c11 /O2 /c ae3_loop_leaves.c /Fobuild\ae3_loop_leaves.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /O2 /c ae3_resumable_leaves.c /Fobuild\ae3_resumable_leaves.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /O2 /c ae3_call_leaves.c /Fobuild\ae3_call_leaves.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /O2 /c ae3_stack_leaves.c /Fobuild\ae3_stack_leaves.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /O2 /c ae3_native_dispatch.c /Fobuild\ae3_native_dispatch.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /O2 /c ae3_guest_execution.c /Fobuild\ae3_guest_execution.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /O2 /c ae3_elf_image.c /Fobuild\ae3_elf_image.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_boot_prefix.c /Fobuild\ae3_boot_prefix.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_boot_memory_fill.c /Fobuild\ae3_boot_memory_fill.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_boot_thread_setup.c /Fobuild\ae3_boot_thread_setup.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_boot_kernel_bridge.c /Fobuild\ae3_boot_kernel_bridge.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_boot_init_prefix.c /Fobuild\ae3_boot_init_prefix.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_boot_probe.c /Fobuild\ae3_boot_probe.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_boot_semaphore_init.c /Fobuild\ae3_boot_semaphore_init.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_ee_sema_kernel.c /Fobuild\ae3_ee_sema_kernel.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_ee_syscall_kernel.c /Fobuild\ae3_ee_syscall_kernel.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_guest_find_address.c /Fobuild\ae3_guest_find_address.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_boot_hook_init.c /Fobuild\ae3_boot_hook_init.obj
if errorlevel 1 exit /b 1
clang-cl /nologo /std:c11 /W4 /WX /O2 /c ae3_ee_memory_bus.c /Fobuild\ae3_ee_memory_bus.obj
if errorlevel 1 exit /b 1
echo [AE3] Compiled x64 object files (NOT a playable executable).
exit /b 0