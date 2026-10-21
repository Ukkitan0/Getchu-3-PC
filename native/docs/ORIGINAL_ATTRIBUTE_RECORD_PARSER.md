# Ape Escape 3 USA — original 0x2BA584 attribute record materialization

Owner-private original USA `SCUS_975.01` ELF SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No original executable or copyrighted game assets are committed.

## Recovery: real parser header and repeated 0x34 records

The source 0x2BA584 parser takes an already preprocessed array of line
strings from 0x2BA3AC. It loads element 1 at 0x2BA5E8, tokenizes it
with ':' via 0x1F28C4, then reads token 0 using 0x428C48 at 0x2BA6A4
and writes the integer to original parser owner +0x1C (0x2BA6C8).
It reads token 3 through 0x428C48 at 0x2BA6F0 and stores that
integer at owner +0x20 (0x2BA6F8). The child count is read from
owner +0x1C; when zero, it skips the child allocation loop.

For each child index i, the original uses preprocessed line-string
entry i+2, calls 0x1F28C4 with the same ':' delimiter and requests
**0x34 bytes** of game memory at 0x2BA714..0x2BA718. Child record
source facts, distinct from native host layout:

| Original offset | Original operation | Input |
|---|---|---|
| +0x00 | SB at 0x2BA818 | first byte of token 0 |
| +0x04 | 0x627718 original string assignment at 0x2BA858 | token 2 |
| +0x08 | SW at 0x2BA8B0 after 0x428C48 | decimal token 1 |

The child key at +4 is looked up in the original red-black tree.
At 0x2BA9C0, the tree node's +0x14 field receives a **pointer to
the 0x34-byte child record**. This is a correction to earlier docs
that described the tree value as a direct source-relative offset.
The numeric and string getters later follow that pointer and read
child +0x08 as the offset into a loaded value allocation.

## Native implementation

`original_attribute_record_parser.hpp` implements the original header
and repeated child-field extraction from **already-tokenized source
lines**, with strict host bounds checking and explicit record structs.
It rejects unavailable rows, malformed headers, missing child bytes,
and negative counts instead of triggering the original unchecked
unsigned loop's unsafe behavior.

`originalAttributeBindDecodedChildren_2ba9c0` exposes an **explicit
host-level indirection collapse**: child key -> child +0x08 source
offset. This matches the *observed lookup* API recovered previously,
but does not claim that the host tree's value is a retail child pointer,
that native string objects have 32-bit guest layouts, or that the
entire original 0x34-byte child has been reconstructed.

## Verification

`verify_original_attribute_record_parser.py` SHA-pins the **entire
406-word source function (0x2BA584..0x2BABDC)**, independently checks
**44 exact opcodes and 16 call edges**, confirms header and child
field stores, 0x34 allocation, zero-child skip and loop back-edge,
real pointer assignment to tree +0x14 and trailer numeric calls.
Passed against the exact private original retail ELF.

`original_attribute_record_parser_test.cpp` covers header fields,
child field extraction, malformed input, zero count, negative count
rejection, oversized counts, 2,000 repeat cases, multiple different
children, duplicate-key replacement, and the original audio strings
and volume getter using source offsets. Local GCC Release, Clang,
UBSan and ASan+UBSan passed against the actual original tree/attribute
code already recovered.

## Completed original trailer field recovery

Original 0x2BA9F8 reads the penultimate preprocessed line at
end-8 bytes and 0x2BAAC4 reads the last line at end-4 bytes.
Both are split on ':' and converted from field 1 through
0x428C48. The penultimate value is written to owner+0x24 at
0x2BAA98. The last value is added to the original owner's
+0x00 base word and stored as owner+0x04 at 0x2BAB60,
using original 32-bit wrapping ADDU semantics.

`original_attribute_record_trailer.hpp` implements these fields
independently from child records. It requires two distinct trailer
lines, checks bounds, preserves negative input and uint32 wraparound,
and provides `originalAttributeStructuredFields_2ba584` to
combine header, child and trailer recovery. The original
0x2BA9F8..0x2BAB7C **97-instruction window** and 15 exact
opcode sites/10 direct calls are SHA-verified against the retail
ELF; the native test passes Release, Clang, ASan and UBSan.
These 97 source words are within the 406 verified above,
not an additional disjoint region.

## Unfinished original behavior

Upstream 0x2BA3AC line filtering and original source-file allocation,
complete parser trailer records (+0x24 and pointer-offset links),
retail actual game data differential tests, original 32-bit string
object representation and guest pointer ownership still need work.
This pass does **not** assert that the source is a retail navrange asset
or that native Ape Escape 3 can boot. It is a source-level C++
reconstruction, not emulation or a substitute game engine.
