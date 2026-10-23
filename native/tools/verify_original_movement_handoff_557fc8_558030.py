#!/usr/bin/env python3
"""Check verified retail register/branch evidence across 557F48..558030.

This does NOT reconstruct the still-untranslated original
557FC8..55802C actor/collision body. Checks existing instruction
pins and that native lifecycle requires 36F83C bit0 regeneration,
preserves original f20/f21 and links into each child CTest.

The retail USA executable is private. Its existing source-verifier
maps embed genuine previously checked 32-bit opcode words; this
handoff check does not claim to re-verify the private executable.
"""
import argparse
import json
import runpy
from pathlib import Path


def check(root: Path) -> dict:
    tools = root / "tools"
    prefix = runpy.run_path(
        str(tools / "verify_original_movement_metric_557f34.py")
    )
    post = runpy.run_path(
        str(tools / "verify_original_movement_post_vector_557f54.py")
    )
    regen = runpy.run_path(
        str(tools / "verify_original_movement_flag0_regenerate_36f83c.py")
    )
    step = runpy.run_path(
        str(tools / "verify_original_movement_post_step_558030.py")
    )

    # Genuine 557F48: MOV.S f20,f0
    # Genuine 557F60: MOV.S f21,f0 (36FB18's returned f0)
    for site, word, dst in (
        (0x557F48, prefix["WORDS"][0x557F48], 20),
        (0x557F60, post["WORDS"][0x557F60], 21),
    ):
        if (word >> 26) != 0x11 or ((word >> 21) & 31) != 16:
            raise AssertionError(f"source {site:#x} not COP1 single float")
        if ((word >> 16) & 31) or ((word >> 11) & 31):
            raise AssertionError(f"source {site:#x} not moving from f0")
        if ((word >> 6) & 31) != dst or (word & 63) != 6:
            raise AssertionError(f"source {site:#x} MOV.S destination differs")

    # The 558030 terminal path still divides source f21/f20.
    terminal_divide = step["WORDS"][0x558060]
    if (terminal_divide >> 26) != 0x11:
        raise AssertionError("original 558060 no longer COP1")
    if ((terminal_divide >> 21) & 31) != 16 or (terminal_divide & 63) != 3:
        raise AssertionError("original 558060 no longer DIV.S")
    # MIPS COP1 instruction encodes ft as divisor, fs as dividend,
    # fd as result. Original f21 / f20 -> f0.
    if (((terminal_divide >> 16) & 31),
        ((terminal_divide >> 11) & 31),
        ((terminal_divide >> 6) & 31)) != (20, 21, 0):
        raise AssertionError("original 558060 wrong f21/f20 registers")

    # The existing original source windows establish the specialized
    # bit0 regeneration and its source ORI of 1 (not 2).
    if regen["WORDS"][0x36F890] != 0x35EF0001:
        raise AssertionError("original 36F83C missing bit0 ORI")
    if regen["WORDS"][0x36F894] != 0xAC8F00A8:
        raise AssertionError("original 36F83C missing flags store")
    lifecycle = (root / "src" / "original_movement_child_lifecycle_557e30.cpp"
                 ).read_text(encoding="utf-8")
    header = (root / "include" / "ae3" /
              "original_movement_post_step_558030.hpp").read_text(
                  encoding="utf-8"
              )
    if lifecycle.count("originalMovementRegenerateFlag0Owned_36f83c(") != 1:
        raise AssertionError("source bit0 regeneration not linked exactly once")
    if "originalF20ResourceMetricBits!=originalF20ResourceMetric" not in lifecycle:
        raise AssertionError("557F48 f20 source continuity missing")
    if "originalF21MovementReturnBits!=originalF21MovementReturn" not in lifecycle:
        raise AssertionError("557F60 f21 source continuity missing")
    if "read_original_flag0_binding_36f83c" not in header:
        raise AssertionError("genuine original source VU0 owner missing")

    cmake = (root / "unreal_bridge" / "CMakeLists.txt").read_text(
        encoding="utf-8"
    )
    import re
    lifecycle_targets = [
        block for block in re.findall(
            r"add_executable\\([^\\n]*\\n[\\s\\S]*?\\)\\n",
            cmake
        )
        if "original_movement_child_lifecycle_557e30.cpp" in block
    ]
    if len(lifecycle_targets) != 6 or any(
        "original_movement_post_step_558030.cpp" not in b or
        "original_movement_flag0_regenerate_36f83c.cpp" not in b
        for b in lifecycle_targets
    ):
        raise AssertionError("one or more original child lifecycle tests not linked")
    return {
        "retail_opcode_evidence": [
            "0x557F48 MOV.S f20,f0",
            "0x557F60 MOV.S f21,f0",
            "0x558060 DIV.S f0,f21,f20",
            "0x36F890 ORI bit0",
            "0x36F894 SW flags",
        ],
        "source_lifecycle_ctest_targets_linked": len(lifecycle_targets),
        "source_register_provenance_checks_present": True,
        "source_flag0_regeneration_required": True,
        "original_557fc8_55802c_collision_body_translated": False,
        "windows_game_playable_verified": False,
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--native-root", type=Path,
        default=Path(__file__).resolve().parents[1]
    )
    print(json.dumps(check(parser.parse_args().native_root), indent=2))
