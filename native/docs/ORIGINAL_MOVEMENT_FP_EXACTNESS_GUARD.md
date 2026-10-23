# Ape Escape 3 USA — original movement finite-float exactness boundary

## Purpose and original-source scope

The native original-engine translation intentionally refuses arithmetic it cannot
yet establish as bit-correct on the original PS2. It must **not** silently call
host IEEE-754 rounding a verified game result. This file records a corrective
audit of that rule for the already recovered original function windows:

- `0x317B60..0x317B90` resource-metric VF1 squared norm, including
  `0x317B68` and `0x317B6C` VADD operations;
- `0x36FB70..0x36FC24` positive-movement VF1 norm, particularly
  `0x36FB80/0x36FB84`;
- `0x36F8C8..0x36F954` original movement-vector regeneration with
  VU0 accumulator `VMADDA` and `VSUBA` operations.

The private original USA `SCUS_975.01` reference SHA-256 remains
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No original executable or commercial resource bytes are in this change.

## Corrected validation defect

Prior exactness checks compared a binary32 operation against the same
arithmetic promoted to binary64:

```cpp
static_cast<double>(result) == static_cast<double>(a) + static_cast<double>(b)
```

That comparison **does not prove** that the binary32 sum is mathematically
exact when nonzero input magnitudes are extremely different. Binary64 itself
can discard the smaller operand. For example, a binary32 `1.0f` and
`0x1p-70f` are both normal, but both host float and host double return
`1.0` after addition; the actual sum is not exactly representable in
binary32 and the original PS2 rounding outcome remains unestablished.

For finite normal binary32 inputs, each nonzero significand has no more than
24 bits. If unbiased binary exponents differ by **more than 24**, a nonzero
small operand cannot contribute to an exactly representable binary32 sum.
If the exponents differ by **at most 24**, the exact two-input sum fits inside
binary64's 53-bit significand, so the existing binary64 comparison is
sufficient for the ordinary finite subset. Subtraction obeys the same rule.

The three native functions listed above now reject large-exponent-gap
additions **before** the existing binary64 equality check, and `0x36F8C8`
does likewise for VU0 accumulator subtraction. This corrects false
exactness claims without guessing the PS2's rounding or special FP modes.
It does not add emulator execution or replacement game physics.

## Regression cases

The existing native test files now include the following adversarial cases:

| Native test file | Input | Required result |
| --- | --- | --- |
| `original_manager_resource_vu0_317b34_test.cpp` | squared norm `1 + 2^-70` | `std::domain_error` |
| `original_movement_flagged_36fb5c_test.cpp` | squared norm `1 + 2^-70` | `std::domain_error` |
| `original_movement_regenerate_36f8c8_test.cpp` | progress `2^-35`: cube `2^-105`, square `2^-70` | `std::domain_error` |
| `original_movement_regenerate_36f8c8_test.cpp` | original `0x36F934` VSUBAw `2.625 - 2^-70` | `std::domain_error` |

Each regression forces a case previously liable to pass the double check
despite losing a nonzero operand. GCC and Clang C++20 compilation plus
UndefinedBehaviorSanitizer were run for a **standalone equivalent guard
check**, not for the complete changed repository. The actual three
production test targets, full CMake/CTest suite, original ELF verification
in this session and Windows/Unreal build remain **unverified**.

## Remaining original source work

This pass is source-integrity hardening, **not** a playable movement frame.
To increase native movement coverage without inventing results:

1. Validate original EE COP1 versus VU0 per-instruction FP behavior and
   establish a source-backed normal-range rounded implementation.
2. Recover the repeated `0x36FB70 -> 0x36FC18 -> 0x36F8C8` loop with
   genuinely owned VU0 registers and original child status/flag commits.
3. Complete the subsequent `0x557F54+` animation/collision actor updates
   and real Windows runtime integration.

Until then, inexact and unsupported branches must continue to refuse
local calculation or invoke the genuine game-owner callback. No guest data,
actor position or collision result may be fabricated.
