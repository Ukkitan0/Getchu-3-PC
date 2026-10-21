# Ape Escape 3 USA — original navrange local centroid projection

Private retail original SCUS_975.01 SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No original game executable or asset bytes are committed. Native C++ only.

## Original retail call chain and data flow

The original navrange resource registration scans polygons at 0x21F6D4
and passes original group/header vectors and each polygon centroid
to 0x21EF3C. At 0x21EF54, original A0 (resource header +0x20)
is saved in S7; at 0x21EF64, A3 (polygon centroid) is saved in S1.

0x21F00C transfers S7 to VF3; 0x21F014 transfers S1 to VF1;
0x21F020 VSUB.xyzw computes the complete four-lane vector difference
(centroid minus header+0x20). The original copies the difference back
to S1 and restores it into VF3 at 0x21F048.

Four LQC2 instructions at 0x21F030, 0x21F04C, 0x21F060, 0x21F070
load columns 0,1,2,3 of the previously recovered 0x359F44
**inverse quaternion matrix** through the original V0 output pointer.
Original VMULAx/VMADDAy/VMADDAz/VMADDw at
0x21F050, 0x21F054, 0x21F068, 0x21F074 compute the
four-lane matrix product. This runs on the actual navrange
polygon-eligibility path, not just its optional output path.

## Native source-level C++ recovery

`originalNavrangeLocalCoordinate_21f020_21f074` implements
the source vector subtraction and matrix-column multiplication.
`originalNavrangeLocalCentroidFromFrame_21efa4_21f074`
connects the previously recovered original resource-frame +0x20 vector,
quaternion rotation matrix, matrix inverse, and centroid to compute this
source-space intermediate. The source +0x40 vector is not treated as
an arbitrary radius or extent. The predicate result is NOT invented.

## Verification and limits

`native/tools/verify_original_navrange_local_coordinate.py` checks
17 original retail opcode anchors, source VU register transfers,
all four matrix-column loads, and the two original matrix calls.
The private-ELF verification passed locally.

The native regression covers identity, an inverse quarter-turn,
general homogeneous multiplication and 1000 deterministic
centroid-minus-origin tests. Local optimized strict C++20, UBSan and
ASan+UBSan passed using a test-only compatible frame API.
The complete native game build has not yet been executed.

Original VU0 rounding remains approximate because the current
inverse implementation uses mathematically equivalent cofactors
rather than bit-exact original VU scheduling.

Still unfinished: computation of the original F20 eligibility scalar
through 0x21EF3C, original retail navrange resource allocation,
the game actor update, rendering and Windows boot.
No GitHub Actions were dispatched.
