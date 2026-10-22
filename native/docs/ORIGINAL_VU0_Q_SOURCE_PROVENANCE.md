# Ape Escape 3 USA — caller-owned VU0 Q provenance and native fallback contract

Source: owner-private original `SCUS_975.01`, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

## Original machine evidence

The fallback at `0x57CA68` reads the VU0 Q register through
**four VMULq** instructions in the initial and repeated
candidate bodies, at `0x57CB9C`, `0x57CBB8`,
`0x57CC94` and `0x57CCB0`.

The original routine contains **no VDIV, VSQRT or CTC2 Q
initializer**. Q is global caller-owned COP2 state.
This is separate from both the EE game RNG and VU0
`R` register side effects previously recovered.

The source-auditor `native/tools/audit_original_vu0_Q_state.py`
scans all original 1,351,830 .text machine words:
- 781 in-text VU0 `VMULq` sites.
- Exactly two direct `CTC2 Q` writers at
  `0x41DB9C` and `0x41DBB0`, in a different subsystem.
  Their surrounding immediate loads correspond to
  `0xBFC90FDA` and `0x3FC90FDA`.
- One direct `CFC2 Q` reader at `0x41DBBC`.
- One macro `VDIV` at `0x3BA4F0` and one
  macro `VSQRT` at `0x3BA3DC`.
- No Q write in the fallback body itself.

The counts are audited against the actual source, not
proof of how the global Q value evolves at every frame.
Other VU microprogram effects or scheduler context
must be considered when the full original engine runs.

## Corrected native C++ interface

`originalActorFallbackSelect_57ca68` now takes an
**explicit 32-bit original VU0 Q bit pattern**, in
addition to its two independently loaded source vectors:

```text
source polygon record -> original fallback $a0
owner+0x14->+0x40     -> original fallback $a1
controller+0x20       -> original fallback $a2
owner-original VU0 Q  -> source VMULq state
owner-original MT19937 -> original EE RNG state
owner-original VU0 R   -> optional source VRINIT/VRXOR state
```

The original-specific VU0 candidate transform receives Q
unchanged, not an invented default like 1.0.
`OriginalActorNavigationPipeline_57c7cc` and its stage
wrapper provide `setOriginalFallbackQBits_57ca68` and
reject missing Q whenever the native fallback candidate
transform is attached. This intentionally prevents a
subtly incorrect, apparently working movement port.

The existing source-regression tests now assert exact
pass-through of distinct owner vector / controller vector
and original Q bit values, and verify missing inputs are
rejected. The isolated original six-candidate native sampler
passed GCC and Clang+ASan/UBSan against all 59 genuine
owner-private `woods_c` navigation polygons. The complete
repository and Windows original game boot remain unverified.

The unresolved element is the *actual* VU0 candidate
vector arithmetic, not the RNG or input argument
provenance. No replacement gameplay engine or emulator
was added; no copyrighted game assets are committed and
no GitHub Actions were triggered.
