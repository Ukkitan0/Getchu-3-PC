# Original Ape Escape 3 USA actor projection ray and VU0 XYZ dot

User-owned retail `SCUS_975.01`, exact SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

## Two source-backed recovered native calculations

**Complete original `0x4748AC..0x4748CC` XYZ dot product**:

Original source VU0 instructions `0x4748B4` (VMUL.xyzw),
`0x4748B8` (VADDy with mask x,y,w, preserving original Z)
and `0x4748BC` (VADDz) yield
`(A.x*B.x + A.y*B.y) + A.z*B.z`.
The original component W is explicitly unused.

This source calculation is called by original `0x21EE00`
and `0x57CA68`, both part of original actor navigation geometry.
The native implementation is `originalVU0DotXYZ_4748ac` in
`original_actor_ray_geometry.cpp`. It uses ordinary direct
host-native float32 arithmetic, not an emulated VU.

**Original `0x4C1B3C..0x4C1BA4` ray setup**:

The original actor-projection front end performs original
`VSUB.xyzw` of two passed vectors, computes the dot of their
XYZ displacement with itself, calls original `SQRT.S` to find
the length passed to source `0x21BD3C` in `$f12`,
reads the exact retail 1.0f constant at `0x6D0194`,
and multiplies **all four displacement components**
by its reciprocal to obtain the original normalized ray.

The new native `originalActorProjectionRay_4c1b3c`
returns original displacement, original source segment length,
and its normalized direction. It deliberately does not
invent a missing "distance zero" policy; original floating
operations are preserved for degenerate input.

The original polygon traversal/inters ection `0x21BD3C`
is **not implemented**. This is a verified native source
frontend for that routine, not a claim that actor collision
or navigation projection is complete.

## Verification

`native/tools/verify_original_actor_ray_geometry.py`
validates 36 retail source instructions in both native
operation spans, eight independently decoded VU0 operations,
16 retail instruction/constant anchors, and the source
JAL from 0x4C1BA8 to 0x21BD3C.

`native/tests/original_actor_ray_geometry_test.cpp`
validates exact synthetic dot operands, displacement W lane,
length, squared normalized XYZ length, and **all 59 original
woods_c polygon vertex pairs** from original
`woods_c/navigation.pck` when its private path is supplied.
GCC Release, Clang Release and UBSan passed locally.

No original proprietary ELF/ISO/assets are committed.
No PlayStation emulation, fake gameplay, or workflow dispatch
is included. Full native Windows boot remains unachieved.


## 2026-10-21 — original first vs later ray source A1 provenance

Source `0x57C7E4 PCPYUD s5,zero,a1` saves a 128-bit packed copy of incoming `a1` with **X/Y zero, Z/W retained**. At `0x57C814 JAL 0x4C1B3C`, original `a1` is still *unmodified*: the first ray uses all four incoming lanes. After selecting another target, source `0x57C88C PCPYUD a1,zero,s5` runs before the `0x57C890` second ray call (delay slot `0x57C894 PCPYUD a2,zero,v0`). Therefore subsequent rays use **masked source a1** and **masked target a2**, not the initial full `a1` vector.

The new direct native `originalActorPackedSubsequentRayStart_57c88c` function performs precisely this source packing. `OriginalActorNavigationPipeline_57c7cc::call_original_4c1b3c_project` selects full incoming A1 for the initial ray and the masked version after the first genuine target dispatch. Full 128-bit QMFC2/QMTC2 semantics from the preceding target-vector recovery remain preserved. This is source operand recovery, not replacement steering or collision.

Exact source opcode words `0x57C7E4`, `0x57C814`, `0x57C88C`, `0x57C890`, and `0x57C894` were locally verified against the genuine private USA executable. Standalone GCC 14.2 optimized and Clang 17 ASan/UBSan tests of the VU0 input-packing/projection math passed against **all 59** owner-private `woods_c/navigation.pck` triangles. The full native Windows application remains unbuilt/unbooted.
