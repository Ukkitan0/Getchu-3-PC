# Original AE3 USA fallback 0x57CA68: source operand provenance fix

The original `SCUS_975.01` function `0x57C8EC` calls fallback
`0x57CA68` with **three** source arguments: polygon pointer in
`$a0`, owner-provided packed vector in `$a1`, and controller
`+0x20` packed vector in `$a2`.

The previous native interface incorrectly passed only the
controller+0x20 vector under the name `originalInputA1`.
It silently omitted a full 16-byte source input, so even a
correct future VU0 candidate calculation would have been wrong.

## Source audit (actual owned USA executable)

User-private original SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

- `0x57C970` loads the controller's owner pointer from +0x04.
- `0x57C974` loads the 16-byte controller vector **+0x20**
  into original **`$a2`** (`0x7A060020`).
- `0x57C978` follows that owner's pointer at **+0x14**.
- `0x57C97C` calls original fallback **`0x57CA68`**.
- Its **delay-slot instruction** at `0x57C980` loads the
  actor owner's packed **+0x40** vector into original **`$a1`**
  (`0x79E50040`).
- Inside the fallback, MMI packed-copy instructions at
  `0x57CA78` and `0x57CA88` extract the high halves of
  **both** original `$a1` and `$a2` into separate registers.
  This proves the original computation uses distinct source data.

`native/tools/verify_original_actor_fallback_operand_chain.py`
SHA-verifies the private USA original and seven exact machine
words, checking opcodes, native register indices and original
JAL/delay-slot pairing.

## Native C++ fixes

`originalActorFallbackSelect_57ca68` and the original
candidate-transform interface now explicitly require
both source vectors:

```text
originalOwner14Vector40 -> fallback $a1
originalController20   -> fallback $a2
```

The actor pipeline now has
`setOriginalFallbackOwnerVector_57ca68`; the stage
wrapper exposes the same setter. Supplying the original
fallback-transform object without first supplying the
owner source vector causes a clear native exception.
**The controller+0x20 vector is never used as a guessed
substitute**.

The existing regression tests have been updated to use
distinct tagged buffers, assert that the fallback callback
receives both unchanged, and verify the missing-owner case.
They also retain checks for six original candidates,
12 EE RNG draws and 12 VRINIT/12 VRXOR source operations.

This still is **not a complete actor movement port**.
The native transform of VU0 `0x57CB4C..0x57CCB0`
must use both packed source vectors and the caller-owned
original VU0 `Q` state. Neither the actual Q provenance
nor the complete vector transform has been recovered.
The original game has not yet booted as a native Windows
game. No emulator and no proprietary assets are committed;
no GitHub Actions were manually triggered.
