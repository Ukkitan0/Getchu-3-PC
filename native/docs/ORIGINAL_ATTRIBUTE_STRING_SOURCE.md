# Ape Escape 3 USA — Original 0x2BB3BC string attribute recovery

Source: owner-private original USA SCUS_975.01 ELF, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No commercial binary or game assets are committed.

## Recovered source behavior

The original 0x1BB9A8 audio initializer calls 0x2BB3BC five times
for `name`, `midi`, `reverb`, `vb`, and `vh`. The getter receives
an output 32-byte string object, a loaded source-data pointer, an
attribute-name C string and a caller-supplied 32-byte fallback string.

0x2BB3BC:
1. Copies the fallback object's two 16-byte parts and constructs a
   temporary search name via original 0x627128.
2. Finds the key through original ordered index 0x5419EC.
3. On a hit, 0x54204C extracts the source-relative value offset
   from the resolved node descriptor at +0x08 and adds the loaded
   allocation's base address.
4. 0x35AB88 constructs a string object from the source C string.
   The original implementation has small/large string branches;
   source-level host C++ owns the decoded characters instead.
5. 0x2BB1AC compares source string to literal `EmptyStrings32`
   (original address 0x6F8E38) via original strcmp 0x42C44C,
   converting equality to a boolean. Original 0x2BB468 BEQZ
   selects source when the string is **not equal** to the marker.
   Otherwise, or when the key is absent, it selects the
   caller's original fallback string.
6. Temporary source and search string objects are released.

## Native implementation

`native/include/ae3/original_attribute_string.hpp` provides
`originalAttributeString_2bb3bc`, with original key lookup, bounded
NUL-terminated value reads, exact marker/default behavior, and
host-owned strings. It deliberately rejects unterminated/truncated
source data rather than reading outside the loaded allocation.

`OriginalAudioDecodedAttributeSource` implements BOTH the
recovered original string path and the previously recovered
`0x2BB1D0` float path from the SAME decoded source index.
`originalAudioDescriptorFromDecodedAttributes_1bb9a8` feeds that
source into the earlier real audio field initializer. No manual
mock provider is needed for five strings and `volume_scale`,
provided a decoded source index and bytes are available.

The previous `OriginalAudioNumericAttributeBridge` stays
unchanged for callers using a separate string provider.

## Source verification and tests

`verify_original_attribute_string.py` checks the retail ELF SHA,
**six entire code regions totaling 342 original instructions**,
**44 independently pinned instruction words**, **15 call edges**,
four original branch targets, 32-byte fallback and source-copy
operands and exact `EmptyStrings32` literal.

`Getchu3OriginalAttributeStringTests` tests five audio string
fields and numeric integration, original defaults and marker,
empty strings, long strings, embedded NUL, bounded-memory errors,
signed-zero and missing-word numeric behavior and 1,500
deterministic source names. It passed local optimized C++20,
UBSan and ASan+UBSan against the actual implementation.

## Unfinished source dependencies

Original serialized tree DESERIALIZATION and its allocations,
retail resource-file ownership, and exact guest 32-byte string
object memory layout are NOT recovered. This is source-level
native string data selection, not a bit-exact PlayStation 2
runtime or a substitute sound engine. Full Windows game
build not tested. Native Ape Escape 3 remains nonplayable.
No GitHub Actions were manually triggered.
