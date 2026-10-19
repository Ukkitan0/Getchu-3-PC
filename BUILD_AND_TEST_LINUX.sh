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
# Link the verified shared objects instead of recompiling ~4 MB of generated C.
cc -std=c11 -O0 ae3_native_dispatch.c test_dispatch.c -L. -lae3_scalar -lae3_control -lae3_loop -lae3_resumable -lae3_calls -lae3_stack -Wl,-rpath,'$ORIGIN' -o ae3_dispatch_test
./ae3_dispatch_test
echo 'All native C candidate translator tests passed (NOT a playable game).'