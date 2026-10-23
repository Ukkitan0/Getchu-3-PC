#!/usr/bin/env python3
"""Check exact binary32 VU0/COP1 ADD/SUB rejection guards.

No proprietary game files, replacement physics, or PS2 emulation.
The already recovered original SCUS_975.01 source windows contain
VMADDA/VSUBA/VADD instructions. The native source subset accepts
ordinary inputs only when binary32 results are mathematically exact.
A binary64 equality alone misses tiny but nonzero addends.

This deterministic verifier checks the four native guard sites and
compares the accept/reject rule against exact rational arithmetic.
It does NOT prove PS2 rounding for refused inputs, run original
gameplay, or replace compiler-backed CTest.
"""
import argparse
import fractions
import json
import math
import random
import struct
from pathlib import Path

SOURCES = (
    "original_manager_resource_vu0_317b34.cpp",
    "original_movement_flagged_36fb5c.cpp",
    "original_movement_regenerate_36f8c8.cpp",
    "original_movement_post_vector_557f54.cpp",
)
EXPECTED_COUNTS = (1, 1, 2, 1)


def source_guards(root: Path):
    findings = {}
    for name, count in zip(SOURCES, EXPECTED_COUNTS):
        data = (root / "src" / name).read_text(encoding="utf-8")
        found = data.count("std::abs(std::ilogb(")
        if found != count or data.count(")>24)") < count:
            raise AssertionError(
                f"{name}: expected {count} exponent-gap guards, got {found}"
            )
        findings[name] = found
    return findings


def ordinary32(bits: int) -> float:
    return struct.unpack("<f", struct.pack("<I", bits))[0]


def to_binary32(v: float) -> float:
    try:
        return struct.unpack("<f", struct.pack("<f", v))[0]
    except OverflowError:
        return math.copysign(math.inf, v)


def normal_or_zero(v: float) -> bool:
    return math.isfinite(v) and (v == 0.0 or abs(v) >= 2.0 ** -126)


def native_exact_add_guard(a: float, b: float, subtract: bool) -> bool:
    if subtract:
        b = -b
    # C++ std::ilogb yields frexp exponent minus one on normals.
    if a != 0.0 and b != 0.0:
        if abs(math.frexp(a)[1] - math.frexp(b)[1]) > 24:
            return False
    result = to_binary32(a + b)
    return normal_or_zero(result) and float(result) == (a + b)


def ideal_exact_add(a: float, b: float, subtract: bool) -> bool:
    if subtract:
        b = -b
    result = to_binary32(a + b)
    return normal_or_zero(result) and (
        fractions.Fraction(result)
        == fractions.Fraction(a) + fractions.Fraction(b)
    )


def run(root: Path, cases: int) -> dict:
    guards = source_guards(root)
    rng = random.Random(0xAE3358)
    accepted = 0
    for index in range(cases):
        numbers = []
        for _ in range(2):
            exponent = rng.randrange(1, 255)
            mantissa = rng.randrange(1 << 23)
            sign = rng.randrange(2)
            numbers.append(ordinary32(
                (sign << 31) | (exponent << 23) | mantissa
            ))
        a, b = numbers
        for subtract in (False, True):
            observed = native_exact_add_guard(a, b, subtract)
            expected = ideal_exact_add(a, b, subtract)
            if observed != expected:
                raise AssertionError(
                    f"false exactness at case {index}, "
                    f"{a=}, {b=}, {subtract=}"
                )
            accepted += observed
    # Magnitude gap exceeds both binary32 and binary64 significands.
    for a, b in ((1.0, 2.0 ** -70), (2.625, 2.0 ** -70)):
        assert not native_exact_add_guard(a, b, False)
        assert not native_exact_add_guard(a, b, True)
    return {
        "original_source_engine": "Ape Escape 3 USA",
        "source_guard_sites": guards,
        "exact_rational_add_sub_comparisons": cases * 2,
        "accepted_exact_ordinary_operations": accepted,
        "adversarial_inexact_lost_operand_cases": 4,
        "binary32_guard_matches_exact_rationals": True,
        "full_native_game_compiled_or_playable": False,
    }


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--cases", type=int, default=20000)
    ap.add_argument("--native-root", type=Path,
                    default=Path(__file__).resolve().parents[1])
    args = ap.parse_args()
    if not 1 <= args.cases <= 1000000:
        ap.error("--cases must be between 1 and 1000000")
    print(json.dumps(run(args.native_root, args.cases), indent=2))
