# Ape Escape 3 USA — Original parsed attribute record count/stride accessors

Original executable: user-owned USA `SCUS_975.01` SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

## Retail source recovery

The original 0x2BB03C attribute loader writes its global owner
state at **0x7406A8**, then calls 0x2BA218, 0x2BA3AC and
0x2BA584 to prepare the source attributes.

These source owner fields have now been recovered together:

| Original owner offset | Written by | Read by |
|---|---|---|
| +0x04 | 0x2BAB60, rebased 32-bit base pointer | 0x2BB100 |
| +0x20 | 0x2BA6F8, fourth integer field from header | 0x2BB0E8 record count getter |
| +0x24 | 0x2BAA98, penultimate trailing row integer | 0x2BB0FC record stride |

**Important clarification:** owner+0x24 holds an indexing **stride**,
not the same count as owner+0x20. The native
`OriginalAttributeRecordTrailer_2baa90` previously preserved the
numeric owner word but did not yet expose its use by the accessor.

Original 0x2BB0F4..0x2BB10C indexes a source record by loading
the pointer and stride, then performing the R5900 32-bit
`MULT a0,t6,a0`, returning `base + product` in the
0x2BB108 `JR` delay slot:

```cpp
uint32_t guestAddress = base + uint32_t(index) * uint32_t(stride);
```

The same 32-bit wraparound behavior is retained in the native
math. There is no fabricated record stride, guest pointer, or
payload schema. An actual loaded original allocation is still
needed to dereference the computed address.

## Implementation and native safety

`original_attribute_record_index.hpp` implements:
- `originalAttributeRecordIndexFromParser_2bb0f4`, connecting
  source owner fields already reconstructed from actual original
  control flow to the record index layout;
- `originalAttributeRecordGuestAddress_2bb0f4`, preserving exact
  32-bit source address arithmetic;
- `originalAttributeRecordBytesChecked_2bb0f4`, a **separate
  native safety adapter** accepting an externally owned loaded
  buffer and its original guest-base address. It requires the
  requested index to be within the source record count and the
  requested bytes to fit within the provided allocation.

The original 0x2BB0F4 accessor itself has no host bounds checks;
the checked adapter intentionally does not claim bit-exact error
behavior or actual game guest-memory allocation.

## Verification

`verify_original_attribute_record_index.py` verifies the exact
original ELF SHA and SHA of the entire **16-word original**
0x2BB0D0..0x2BB10C pair of functions, **13 exact opcode
anchors**, the original count getter and pointer/stride loads,
MULT register fields and ADDU return, and source writer sites
at 0x2BA6F8/0x2BAA98/0x2BAB60.

`Getchu3OriginalAttributeRecordIndexTests` exercises source
record addressing, 32-bit wraparound, negative stride, bounds
rejection, a source-pointer-based native buffer read, and a
complete synthetic original source-text -> parsed owner fields
-> indexed record calculation. Optimized GCC Release,
Clang, UBSan and ASan+UBSan passed locally. Git blobs match the
tested source and tool files exactly by SHA-1.

## Still missing

Original retail source-file allocation, actual string-object
representation and stage data, full native actor/renderer
execution and Windows boot are not implemented. This is
source-derived native pointer/stride mathematics, not PS2
emulation. The original game remains **nonplayable on Windows**.
No GitHub Actions were manually triggered.
