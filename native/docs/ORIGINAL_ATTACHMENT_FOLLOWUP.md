# Original Ape Escape 3 — expanded Monkey attachment decompilation

Source: owned original USA `SCUS_975.01` SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

The goal is to recover the original C++ engine from its real MIPS executable, **not** develop replacement gameplay. There is still no playable native Windows build.

## Complete small original methods and conditional branches now translated

| Retail virtual method | Original behavior now in host-native C++ |
| --- | --- |
| `0x56B3B4` | If byte at object +0x3D is nonzero, store argument or original owner+4 fallback at +0x40 and return 2; otherwise return 0 |
| `0x56B3E4`, `0x56B3EC` | Return original third argument; no-op |
| `0x56BA98`, `0x56BAA0`, `0x56BAA8` | Return zero, pass through third argument, no-op respectively |
| `0x56B9A8` | Call real `0x313A88(this+0x14)` and return 2 if true or zero otherwise; native implementation retains this unresolved original call as an explicit interface |
| `0x56B28C` | If original owner field +0x34 is nonzero, call genuine `0x3102D4`; return 2 in both cases |
| `0x56B9D0 / 0x56BA00` | Input flag bits 0 or 1 select direct success; conditionally forward to original `0x27679C`, then clear object byte +0x3C |
| `0x56BFE8`, `0x56C020`, `0x56C054` | Forward to child virtual slots 2, 4, 3 (respectively) when child at object +8 exists; use correct no-child return behaviors |
| `0x315DB0` | Mark the owned child record at +0xA1, call original allocator free `0x41A3B0`, and clear owner child pointer |

## Real original timing and mode transitions within `0x56AF44`

- At `0x56AF70–0x56AF80`, compare actual float at attachment +0x54 against **zero**; only zero (including negative zero) follows the initial phase. NaNs enter the other path.
- At `0x56B000 / 0x56B2A8 / 0x56B2C4`, actual mode field +0x50 selects owner-data table virtual addresses: **1 → 0x707110**, **2 → 0x707130**, **3 → 0x707150**, otherwise **0x707170**. These original ELF addresses are identifiers, NOT Windows pointers or recreated vector data.
- At `0x56B07C–0x56B09C`, the code adds the actual runtime frame-delta global `0x635488` to attachment float +0x54 and writes the result back.
- At `0x56B094`, `0x56B160` and `0x56B188`, the real ordered floating-point comparisons establish timer control phases at **0.3**, **0.9**, and **1.4** in original game time units. Vector/effect side effects within each phase are NOT reconstructed.
- At `0x56B28C`, an optional game call depends on original owner+0x34; both paths return 2.

Original long method `0x56AF44` has **21 verified direct JAL instructions targeting 16 unique original callees**. They include `0x276234`, `0x276538`, `0x276580`, `0x276B68`, `0x2767F4`, `0x2773D4`, `0x36DA14`, `0x36DBF4`, `0x276598`, `0x2762BC`, `0x276274`, `0x3103F0`, `0x3102D4`, `0x441F40`, `0x30FD54`, and `0x30FDCC`. Identifying them does not mean porting them.

## Validation

`native/tools/verify_original_attachment_followup.py` verifies SHA-256 of the original user-owned ELF, **4 virtual tables, 55 exact instruction sites**, original float constants, and all 21 direct JAL sites in the complex method. It writes metadata only.

`native/tests/original_attachment_followup_test.cpp` exercises zero/fallback/set cases, flag branches, four timer phases, NaN, mode-template mappings, exact virtual-method forwarding, missing-child behavior and cleanup ordering.

Local compilation of the source module passed C++20 optimized warning-as-error tests and a separate UBSan build. **Windows CTest and the authentic original game runtime are NOT yet verified.** GitHub Actions was not triggered.

The unresolved dependencies must be decompiled from the original executable and integrated with real Player/Monkey update loops; no dummy game implementations were added.
