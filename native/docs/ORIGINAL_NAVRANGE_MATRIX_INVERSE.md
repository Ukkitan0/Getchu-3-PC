# Ape Escape 3 USA — original 0x359F44 matrix inverse

Source: private original retail `SCUS_975.01`, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Game executable, Lua scripts, and original game assets are not committed.

## Original function completely mapped
Retail `0x359F44..0x35A178` contains **142 original instructions**:
132 COP2 vector operations, four LQC2 source-column loads, four SQC2
destination-column stores, and two scalar GPR operations. This is the
original general 4x4 cofactor/adjugate **matrix inverse**.

The original reads four 16-byte input columns from A1 offsets 0, 16,
32, 48 (VF3,VF2,VF5,VF4), calculates signed cofactors using twelve
VOPMULA plus twelve VOPMSUB vector cross-product operations and VU
multiply-accumulate/subtract, then divides VF0.w (=1) by its computed
determinant at 0x35A14C. The return columns are scaled by Q and
written to A0 offsets 0, 16, 32, 48 (VF1,VF2,VF3,VF4).
The fourth write (VF3 at offset 32) is in the return delay slot.
The destination pointer is returned in V0.

The caller at 0x21EFA4 invokes the previously recovered 0x359E74
quaternion-to-matrix routine, copies all four 128-bit columns at
0x21EFB0..0x21EFD4, and then invokes 0x359F44 at 0x21EFDC.

## New native C++20 code
`originalNavrangeInverseCofactors_359f44` computes the mathematical
cofactor/adjugate inverse on the actual column-major 4x4 layout.
`originalNavrangeInverseQuaternion_21efa4_21efdc` composes the
two recovered functions in verified original caller order using the
already-extracted quaternion input, without a fake resource loader.

**Scope:** The cofactor formula matches the decoded purpose of
the original vector calculation, but this is **not** a byte-exact
translation of all 132 COP2 operations. Original VU0 arithmetic order,
rounding, NaN payloads, divide-by-zero flags and singular results
may differ. There is deliberately no identity-on-failure fallback;
this remains a separate recovery module, **not wired into a production
polygon eligibility decision** until exact input/output parity is proven.

## Verification
Run `python native/tools/verify_original_navrange_matrix_inverse.py
PATH/TO/PRIVATE/SCUS_975.01`. The verifier requires the pinned
private ELF SHA-256, the entire function's source-region SHA-256,
family histogram, cofactor cross-product count, four load/store
offsets, determinant division, reciprocal-Q scaling, JR delay-slot,
original caller's two JAL edges and eight matrix-column copy
instructions. No original binary bytes are published.

`Getchu3OriginalNavrangeMatrixInverseTests` covers identity,
negative determinant, affine shear/translation, quaternion
rotations, the genuine original two-call composition, and singular
input propagation. Local isolated strict optimized C++20,
UBSan, and ASan+UBSan runs passed. Those local builds used an
identical-layout test-only definition of the original VU vector
and quaternion rotation function; a complete Windows/game build
and bit-exact retail source-output differential were not run.

## Remaining native game blockers
Original quaternion/resource extraction in 0x21EF3C, real navrange
resource ownership, polygon world-to-local geometry and exact
eligibility, actor updates, level rendering, and Windows boot.
No PS2 emulator, invented replacement gameplay, or manually
triggered GitHub Actions were introduced.
