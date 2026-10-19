#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
: "${AE3_ELF:=$PWD/SCUS_975.01}"
export AE3_ELF
python3 bulk_lift.py
python3 cfg_lift.py
python3 loop_lift.py
python3 resume_lift.py
python3 call_lift.py
python3 stack_lift.py
python3 audit_boot_entry.py
cc -std=c11 -O1 -fPIC -shared ae3_scalar_leaves.c -o libae3_scalar.so
cc -std=c11 -O1 -fPIC -shared ae3_control_leaves.c -o libae3_control.so
cc -std=c11 -O1 -fPIC -shared ae3_loop_leaves.c -o libae3_loop.so
python3 test_bulk.py 8
python3 test_cfg.py 10
python3 test_loops.py 8
cc -std=c11 -O1 -fPIC -shared ae3_resumable_leaves.c -o libae3_resumable.so
python3 test_resume.py 2 96
cc -std=c11 -O1 -fPIC -shared ae3_call_leaves.c -L. -lae3_resumable -Wl,-rpath,'$ORIGIN' -o libae3_calls.so
python3 test_calls.py 3
cc -std=c11 -O1 -fPIC -shared ae3_stack_leaves.c -L. -lae3_resumable -Wl,-rpath,'$ORIGIN' -o libae3_stack.so
python3 test_stack.py 2
python3 test_branch_call_paths.py
python3 test_backward_structure.py
python3 test_backward_call_paths.py
# Runtime integration: original EE ELF mapping and one selected execution engine.
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c test_elf_image.c -o ae3_elf_image_test
./ae3_elf_image_test "$AE3_ELF"
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_boot_prefix.c ae3_elf_image.c test_boot_prefix.c -o ae3_boot_prefix_test
./ae3_boot_prefix_test "$AE3_ELF"
# Link the verified shared objects instead of recompiling ~4 MB of generated C.
cc -std=c11 -O0 ae3_native_dispatch.c test_dispatch.c -L. -lae3_scalar -lae3_control -lae3_loop -lae3_resumable -lae3_calls -lae3_stack -Wl,-rpath,'$ORIGIN' -o ae3_dispatch_test
./ae3_dispatch_test
cc -std=c11 -O1 -Wall -Wextra -Werror ae3_native_dispatch.c ae3_guest_execution.c test_guest_execution.c -L. -lae3_scalar -lae3_control -lae3_loop -lae3_resumable -lae3_calls -lae3_stack -Wl,-rpath,'$ORIGIN' -o ae3_guest_execution_test
./ae3_guest_execution_test
# First original JAL callee: initialize the genuine upper stack region.
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c test_boot_memory_fill.c -o build/test_boot_memory_fill
./build/test_boot_memory_fill "$AE3_ELF"
# Do not fake the SetupThread syscall: stop with the original syscall inputs.
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c ae3_boot_thread_setup.c test_boot_thread_setup.c -o build/test_boot_thread_setup
./build/test_boot_thread_setup "$AE3_ELF"
echo 'Native translation + real boot prefix, RAM stack-fill callee and syscall setup tests passed (NOT a playable game).' 

# First PS2 BIOS services: trace-aware guest kernel boundary and real first init JAL.
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c ae3_boot_thread_setup.c ae3_boot_kernel_bridge.c \
  test_boot_kernel_bridge.c -o build/test_boot_kernel_bridge
./build/test_boot_kernel_bridge "$AE3_ELF"
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c ae3_boot_thread_setup.c ae3_boot_kernel_bridge.c \
  ae3_boot_init_prefix.c test_boot_init_prefix.c -o build/test_boot_init_prefix
./build/test_boot_init_prefix "$AE3_ELF"
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c ae3_boot_thread_setup.c ae3_boot_kernel_bridge.c \
  ae3_boot_init_prefix.c ae3_boot_probe.c -o build/ae3_boot_probe
./build/ae3_boot_probe "$AE3_ELF"
./build/ae3_boot_probe "$AE3_ELF" --test-kernel-fixture

# Real retail nested CreateSema wrappers, experimental native object registry,
# and first syscall-hook initialization. This is NOT a BIOS or playable game.
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c ae3_boot_thread_setup.c ae3_boot_kernel_bridge.c \
  ae3_boot_init_prefix.c ae3_boot_semaphore_init.c test_boot_semaphore_init.c \
  -o build/test_boot_semaphore_init
./build/test_boot_semaphore_init "$AE3_ELF"
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_boot_semaphore_init.c \
  ae3_ee_sema_kernel.c test_ee_sema_kernel.c -o build/test_ee_sema_kernel
./build/test_ee_sema_kernel
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c ae3_boot_thread_setup.c ae3_boot_kernel_bridge.c \
  ae3_boot_init_prefix.c ae3_boot_semaphore_init.c ae3_ee_sema_kernel.c \
  ae3_ee_syscall_kernel.c ae3_boot_hook_init.c test_boot_hook_init.c -o build/test_boot_hook_init
./build/test_boot_hook_init "$AE3_ELF"
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c ae3_boot_thread_setup.c ae3_boot_kernel_bridge.c \
  ae3_boot_init_prefix.c ae3_boot_semaphore_init.c ae3_ee_sema_kernel.c \
  ae3_ee_syscall_kernel.c ae3_boot_hook_init.c ae3_boot_probe.c -o build/ae3_boot_probe
./build/ae3_boot_probe "$AE3_ELF" --test-kernel-fixture --test-sema-hle

# Guest SetSyscall 0x74 table, authentic handler instructions, explicit KSEG0
# memory mapping, and two actual FindAddress wrapper invocations.
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_syscall_kernel.c \
  test_ee_syscall_kernel.c -o build/test_ee_syscall_kernel
./build/test_ee_syscall_kernel
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c ae3_boot_thread_setup.c ae3_boot_kernel_bridge.c \
  ae3_boot_init_prefix.c ae3_boot_semaphore_init.c ae3_ee_sema_kernel.c \
  ae3_ee_syscall_kernel.c ae3_boot_hook_init.c ae3_ee_memory_bus.c \
  ae3_guest_find_address.c test_boot_syscall_registry.c -o build/test_boot_syscall_registry
./build/test_boot_syscall_registry "$AE3_ELF"

# Shared KSEG0/KSEG1 <-> physical EE RAM map and original guest MIPS FindAddress.
# Optional saved title-screen RAM is reference evidence, never boot-time BIOS.
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_memory_bus.c test_ee_memory_bus.c -o build/test_ee_memory_bus
./build/test_ee_memory_bus
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_guest_find_address.c ae3_ee_memory_bus.c test_findaddress_bus.c -o build/test_findaddress_bus
if [ -f analysis/pcsx2_title_ee_ram.bin ]; then
  ./build/test_findaddress_bus "$AE3_ELF" analysis/pcsx2_title_ee_ram.bin
else
  ./build/test_findaddress_bus "$AE3_ELF"
fi
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c ae3_boot_prefix.c \
  ae3_boot_memory_fill.c ae3_boot_thread_setup.c ae3_boot_kernel_bridge.c \
  ae3_boot_init_prefix.c ae3_boot_semaphore_init.c ae3_ee_sema_kernel.c \
  ae3_ee_syscall_kernel.c ae3_boot_hook_init.c ae3_ee_memory_bus.c \
  ae3_guest_find_address.c ae3_boot_probe.c -o build/ae3_boot_probe
./build/ae3_boot_probe "$AE3_ELF" --test-kernel-fixture --test-sema-hle --test-ee-alias
python3 test_boot_probe_alias.py "$AE3_ELF"