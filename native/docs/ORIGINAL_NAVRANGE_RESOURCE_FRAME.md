# Ape Escape 3 USA — Original navrange resource-frame provenance

Source: the owner's private USA retail `SCUS_975.01` ELF, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No original ELF, retail assets or Lua scripts are included in the repository.

## Real original engine call chain

The 32-byte lookup keys at resource `+0x5C` are **not** the
original polygon-eligibility transform arguments. The actual source
at `0x4E4110..0x4E4120` loads three 128-bit values from the *same*
resource header, then calls `0x21F6D4`:

| Original instruction | Original argument | Resource offset |
|---|---|---|
| `0x4E4110 LQ a3,0x30(s2)` | a3 | +0x30 |
| `0x4E4118 LQ a2,0x40(s2)` | a2 | +0x40 |
| `0x4E4120 LQ a1,0x20(s2)` | a1 | +0x20 |

The +0x20 load occupies the original JAL delay slot.
`0x21F6D4` preserves these 128-bit arguments, resolves a polygon
at `0x21D884`, computes its centroid at `0x21B468`, then
calls `0x21EF3C`. Its actual arguments at that call are:
- a0 = resource +0x20..+0x2F
- a1 = resource +0x40..+0x4F
- a2 = resource +0x30..+0x3F (quaternion)
- a3 = polygon centroid
- t0 = 0 (set in the JAL delay slot)

The original eligibility routine saves these values in s7/s0/s1
and passes s0, the quaternion source, to the recovered
`0x359E74` rotation-matrix routine at `0x21EFA4`.
The original `0x359F44` matrix-inverse call follows at `0x21EFDC`.

## Native recovery

- `original_navrange_source_frame.hpp`: bounds-checked 128-bit
  source-vector extraction using byte-exact little-endian word loads.
  Signed zero and NaN payloads are preserved. It can feed the
  earlier native quaternion-to-matrix-to-inverse chain.
- `original_navrange_group_binding.cpp`: a new source-aware
  overload retaining the previous genuine keyed-owner registry
  and polygon scanner while forwarding all three real resource
  vectors and the original centroid to a required callback.
  Group key and 128-bit header vectors remain separate.
- Original magic and zero-count skips remain no-ops; no eligibility
  default is introduced, and the previous callback remains intact.

The inverse computation remains a mathematical source recovery,
not yet byte-identical to PS2 VU0 exceptional or rounding behavior.
The full original `0x21EF3C` scalar result is not ported and this
new callback must still be implemented to produce native gameplay.

## Verification

Run `python native/tools/verify_original_navrange_resource_inputs.py
PATH/TO/PRIVATE/SCUS_975.01` to re-audit SHA-pinned instructions:
three original LQ offsets, thirteen POR register copies, five JAL
edges, original centroid transfer, and caller t0=0.

Native C++ tests cover raw input bits and 256 deterministic source
patterns, truncated-resource rejection, identity quaternion/inverse,
source-aware callback argument flow, repeated-key reuse, and skip paths.
Optimized C++20, UBSan, ASan+UBSan passed for the parser and isolated
binding tests (the binding used a test-only replica of its existing
C++ API). **The full CMake game build has not been run.**

No PS2 emulator, gameplay substitution, original retail bytes,
or GitHub Actions dispatch was introduced. The game is still not
native-Windows playable; original source resource ownership, full
eligibility, actor update, renderer, and bootstrap remain unfinished.
