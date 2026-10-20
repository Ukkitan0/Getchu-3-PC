#!/usr/bin/env python3
"""Original AE3 USA (SCUS_975.01) entity + controller code routing.

Reads actual original MIPS JAL/J opcodes and Lua API name comparisons from a
hash-verified owner-private ELF. Exports address/name metadata only; does NOT
emulate PS2 hardware or synthesize game logic.
"""
import argparse
import json
from pathlib import Path
from recover_retail_elf_dispatch import RetailElf, analyze, PIN


def direct_target(pc, word):
    if word >> 26 not in (2, 3):
        return None
    return ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)


def routes(elf, rows, end_address):
    result = []
    for i, record in enumerate(rows):
        start = int(record["compare_jal"], 16)
        limit = int(rows[i + 1]["compare_jal"], 16) if i + 1 < len(rows) else end_address
        # Limit output to initial dispatch region, never suggest this is a
        # complete CFG or treat distant unrelated calls as constructors.
        limit = min(limit, start + 0x48)
        direct_calls = []
        direct_jumps = []
        for pc in range(start + 4, limit, 4):
            word = elf.word(pc)
            opcode = word >> 26
            dest = direct_target(pc, word)
            if opcode == 3 and dest != 0x42C44C:
                direct_calls.append({"site": hex(pc), "target": hex(dest)})
            if opcode == 2:
                direct_jumps.append({"site": hex(pc), "target": hex(dest)})
        result.append({
            **record,
            "early_original_jal_calls": direct_calls,
            "direct_original_jump_targets": direct_jumps,
            "needs_deeper_trace": not bool(direct_calls or direct_jumps),
        })
    return result


def verified_routes(path):
    original = RetailElf(path)
    discovery = analyze(original)
    entities = routes(original, discovery["new_ent_type_comparisons"], 0x216F00)
    controllers = routes(original, discovery["c_ctrl_opcode_comparisons"], 0x219BAC)

    def checked(items, name, field, target):
        found = [r for r in items if r["name"] == name]
        if len(found) != 1 or hex(target) not in [x["target"] for x in found[0][field]]:
            raise ValueError("Original ELF MIPS dispatch mismatch for " + name)

    for name, dest in {"Player":0x20DAA4, "Monkey":0x20D70C,
                       "MGSPlayer":0x20DB18, "Cannon":0x20F230,
                       "Button":0x2135A8}.items():
        checked(entities, name, "early_original_jal_calls", dest)
    for name, dest in {"trig_wait":0x218E9C, "weapon_spawn":0x218D38,
                       "monkey_cap_wait":0x218C48, "se":0x2191D4,
                       "onDamage":0x217FE4}.items():
        checked(controllers, name, "direct_original_jump_targets", dest)

    return {
        "format": "getchu3/original-retail-entity-controller-routes/v1",
        "source_sha256": PIN,
        "entity_count": len(entities),
        "controller_count": len(controllers),
        "method": "Static original R5900 MIPS JAL/J target recovery after verified Lua class/command string comparisons",
        "limitation": "First calls include helper/argument parsers, not necessarily the constructor; missing routes require branch and shared-code analysis. NOT a native game implementation.",
        "entities": entities,
        "controllers": controllers,
    }


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("original_elf", type=Path)
    ap.add_argument("--out", type=Path)
    args = ap.parse_args()
    report = verified_routes(args.original_elf)
    output = json.dumps(report, indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(output, encoding="utf-8")
    else:
        print(output, end="")
    print("Verified original class dispatch comparisons:", len(report["entities"]))
    print("Verified original command comparisons:", len(report["controllers"]))
    print("Class routes with early code calls:",
          sum(bool(r["early_original_jal_calls"]) for r in report["entities"]))
    print("Commands with direct handler jumps:",
          sum(bool(r["direct_original_jump_targets"]) for r in report["controllers"]))


if __name__ == "__main__":
    main()
