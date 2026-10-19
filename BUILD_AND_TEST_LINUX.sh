#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
: "${AE3_ELF:=$PWD/SCUS_975.01}"
export AE3_ELF
python3 bulk_lift.py
python3 cfg_lift.py
python3 loop_lift.py
cc -std=c11 -O1 -fPIC -shared ae3_scalar_leaves.c -o libae3_scalar.so
cc -std=c11 -O1 -fPIC -shared ae3_control_leaves.c -o libae3_control.so
cc -std=c11 -O1 -fPIC -shared ae3_loop_leaves.c -o libae3_loop.so
python3 test_bulk.py 8
python3 test_cfg.py 10
python3 test_loops.py 8
cc -std=c11 -O1 ae3_native_dispatch.c ae3_scalar_leaves.c ae3_control_leaves.c ae3_loop_leaves.c test_dispatch.c -o ae3_dispatch_test
./ae3_dispatch_test
echo 'All native C candidate translator tests passed (NOT a playable game).'