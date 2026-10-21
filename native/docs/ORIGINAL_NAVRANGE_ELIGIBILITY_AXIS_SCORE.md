# Ape Escape 3 USA — original 0x21EF3C navrange scalar decompilation

Source: owner-private retail USA `SCUS_975.01`; SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. Original executable and assets are not committed. This is **native C++ source recovery**, not PS2 emulation.

## Traced retail caller

Original `0x21F6D4` obtains actual polygon records through `0x21D884`, derives each centroid at `0x21B468`, then calls `0x21EF3C` from `0x21F750`. The returned F0 is compared **exactly to zero** at `0x21F75C`; nonzero skips list insertion. The previous recovered group registry remains unchanged.

## Recoverable original scalar core

The full original `0x21EF3C..0x21F5A4` routine has 410 instructions. Its initial MMI/COP2 vector transformation and calls `0x441F40`, `0x359E74`, `0x359F44` are **not fully decompiled** yet. Once its transformed F0 coordinate and F1 half-extent enter the COP1 scalar path:

* At `0x21F1AC`, the float accumulator F20 is initialized to zero.
* Each of three axes tests `coordinate < -halfExtent` using `C.OLT.S`; if false, tests `halfExtent < coordinate`. Equality is inside and unordered NaN comparisons are false.
* Negative outliers contribute `(coordinate + halfExtent)^2`; positive outliers contribute `(coordinate - halfExtent)^2`; inside contributes zero. Contributions accumulate in first/second/third axis order.
* `0x21F3FC` returns F20 in F0. When T0 is nonzero, an additional COP2 vector path executes before return; that path is not yet translated.

The **actual original scalar code** now has a host-native C++20 function `originalNavrangeAxisScore_21f20c`. The associated `0x44BD5C` quadword lane extraction and `0x44BD80` masked lane replacement are represented using byte-exact uint32_t lane operations, including signed zero and NaN payload retention.

**Critical scope constraint:** these helpers take *already-transformed* coordinates and half-extents, not raw 32-byte navrange group keys. They must not be wired into the production `OriginalNavGroupEligibility` until the original upstream vector setup, register provenance and retail navrange allocation are decoded. No fabricated default eligibility decision has been added.

## Verification

Run `python native/tools/verify_original_navrange_axis_score.py PATH/TO/SCUS_975.01` against a private original game ELF. This audits **514 words** from three SHA-pinned whole-original-code regions, **44 original exact opcodes**, **7 original conditional-branch targets**, and **16 genuine call edges**. Original bytes are never uploaded.

Native C++ regression test: `Getchu3OriginalNavrangeAxisScoreTests`. Test cases include both bounds, exact edges, three-axis accumulation, NaN unordered comparisons, exact lane bit retention (including negative zero/NaN payload), and invalid index rejection. Strict optimized C++20, UBSan and ASan+UBSan tests were verified locally.

This pass improves genuine ELF-to-native coverage; game startup, live resource ownership, and the complete native Windows executable are still unfinished. No GitHub Actions runs were requested.
