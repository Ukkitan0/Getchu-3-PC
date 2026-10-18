#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
: "${AE3_ELF:=$PWD/SCUS_975.01}"
export AE3_ELF
python3 bulk_lift.py
python3 cfg_lift.py
cc -std=c11 -O1 -fPIC -shared ae3_scalar_leaves.c -o libae3_scalar.so
cc -std=c11 -O1 -fPIC -shared ae3_control_leaves.c -o libae3_control.so
python3 test_bulk.py 8
python3 test_cfg.py 10
echo 'Native C scalar and bounded-CFG translators passed local reference tests.'
