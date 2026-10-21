# Ape Escape 3 USA — actual navrange resource registration

Source: original retail USA `SCUS_975.01`, original ELF SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. Commercial game data and executable remain private.

This pass follows the already reconstructed circular NavGraph owner lists upstream into their **native resource registration**. It is not a new navigation engine and does not emulate a PlayStation 2.

## Original source function 0x4E4060

Original MIPS/R5900 opcode sequence at `0x4E4080..0x4E40A4` loads a 128-bit game literal at `0x6E3F10`, the ASCII name **`navrange`**, and uses **PCEQB, PAND, PCEQW, PPACH** to test the first eight bytes of the resource header. The zero-padding bytes of the constant are masked out. A nonmatching resource exits without creating group owners.

On a match, original **signed** record count comes from resource **+0x54**, with a BLEZ skip for zero/negative values. Its **original 32-bit group-key table pointer** is at resource **+0x5C**. The caller advances through **0x20-byte (32-byte) entries**.

For each source entry:
- `0x36D6F4` performs the original keyed hash-table lookup.
- Only when lookup fails, the game allocates a new list owner via `0x41B800` and initializes it with **`0x21F604`**.
- `0x36D610` inserts the new owner into the source-keyed map.
- `0x21F6D4` traverses the original NavGraph polygons and conditionally populates that owner's circular list.

The original hashing routines internally use the EE multimedia opcodes for 32-byte keys. The native implementation uses byte-exact source key equality to reproduce the **mapping semantics**, not a port of the exact hash-bucket/collision layout.

Other connected original executable windows validated in this batch include original `0x21F5A4` direct list initializer and `0x4E3FEC` owner destructor wrapper. Their native list effects are already represented by the earlier `OriginalNavListOwner` implementation.

## Native code added

`native/include/ae3/original_nav_group_registry.hpp` and `native/src/original_nav_group_registry.cpp` implement:
- Source **navrange** literal matching (including the masked trailing bytes).
- Safe resolution of the original 32-bit pointer to an actual owner-loaded native allocation of 32-byte source keys.
- Reuse of an existing native owner when a key repeats; construction of a new `0x21F604` circular list only for first-seen keys.
- Retention of original source key insertion order and correct per-owner polygon list ordering (new list nodes prepend).
- Forwarding the **actual source key, original polygon and source centroid** into a required `0x21EF3C` predicate callback before insertion. **The huge original COP2/MMI eligibility calculation at 0x21EF3C is not ported** and no arbitrary production eligibility default exists.
- Native owner/list release through previously recovered original `0x21F690` behavior.

The code is integrated into the existing `Getchu3Assets` CMake target with `Getchu3OriginalNavGroupRegistryTests`.

## Verification, including limitations

`native/tools/verify_original_nav_group_registry.py` verifies the SHA-pinned private original ELF: **six complete instruction regions totaling 288 original source words**, **32 independently checked exact instruction sites**, **20 direct original J/JAL call targets**, and the precise original 128-bit `navrange` literal.

Local strict optimized C++20 regression, UndefinedBehaviorSanitizer and AddressSanitizer tests passed. Fixtures exercise:
- The original eight-byte magic match with nonzero trailing bytes.
- Three source keys, including a repeated key resulting in only two allocated owners.
- Exact original source allocation pointer and count resolution.
- Source-specific list population and duplicate prevention across repeated calls.
- Reversed insertion ordering for each list, safe owner teardown.
- Nonmatching magic skip, malformed/missing/overlapping allocations, and signed zero/negative counts.

A private ISO survey decompressed all **1,706 .sz archive entries** successfully, but did **not** find a literal `navrange` string in the uncompressed .sz members. This is a limit: **this particular navrange resource registration path has not been validated on a real retail navrange allocation**, only against source instructions and controlled native data fixtures. The 290 original `.nav` files examined in previous passes are a separate resource type and are NOT substitutes.

## Remaining native port blockers

The real 0x21EF3C eligibility result and its group parameters, original source resource ownership and allocation, live GameObject/Monkey route consumers, movement/update loops, scene drawing/input, and game boot remain unfinished. No complete native Windows game boot was attempted. No GitHub Actions were dispatched.
