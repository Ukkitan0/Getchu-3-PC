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
