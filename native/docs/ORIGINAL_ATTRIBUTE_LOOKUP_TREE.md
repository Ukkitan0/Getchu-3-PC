# Ape Escape 3 USA — original attribute-key tree and numeric getters

Owner-private USA retail source: `SCUS_975.01`, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No copyrighted executable or game resource bytes are committed.

## Recovered original source code

This recovery follows the audio-descriptor reader into its actual key
resolution and primitive value access, without inventing an XML/JSON
parser or other replacement for the original serialized data format.

**Original `0x5419EC` — ordered key-index lookup**

- Reads the original index root from a tree header; each original tree
  node stores its key at `+0x10`, left child at `+0x08`, and right at `+0x0C`.
- At each node, compares the stored key with the requested name using
  `0x627E60`. If node key is less than the query, it descends right;
  otherwise it remembers the node and descends left.
- Checks the candidate's key for exact equality. A missing entry
  returns the original index sentinel.
- `0x627E60` compares string bytes using `0x42B0D8` and breaks equal
  prefixes by length. The native helper uses decoded `std::string_view`
  keys and bytewise-equivalent comparison semantics.

**Original `0x2BB194` — numeric missing marker**

The original reads a field's first 32-bit word and tests it against
`0x7FFFFFFF`. This means `0x00000000`, `0xFFFFFFFF`, negative
zero and ordinary NaN payloads are NOT the same missing-field marker.

**Original `0x2BB1D0` and `0x2BB2B0` — float and integer**

- Both construct the source attribute key, resolve it through the
  original index, obtain the field's **source-relative offset** from
  the resolved descriptor at `+0x08`, and add that offset to the
  original record-buffer base pointer.
- Both honor the `0x2BB194` missing sentinel and return the **caller's
  supplied default** if the attribute is absent or marked invalid.
- On success, the float reader loads the original 32 bits via LWC1,
  preserving the raw IEEE754 value. The integer reader loads the
  original word directly, preserving its bits.
- The native code checks allocation bounds before source word reads;
  original guest memory did not provide these host safety checks.

The existing `0x2BB3BC` string getter is included in the original
instruction audit, along with its `0x2BB1AC` special marker named
`EmptyStrings32`, but its **original 32-byte string-object contents
and allocator semantics have not been reconstructed**.

## Native code and integration

`original_attribute_lookup.hpp` adds the source-derived
`originalAttributeFind_5419ec`, `originalAttributeWord_2bb194`,
`originalAttributeFloat_2bb1d0` and
`originalAttributeIntBits_2bb2b0` functions. It uses explicitly
**already-decoded** tree nodes and a bounded host-native span over
an original loaded source allocation; it does not cast guest pointer
addresses into live host pointers.

`OriginalAudioNumericAttributeBridge` connects the recovered float
path into the previously committed **real audio descriptor** initializer
so `volume_scale` can come from a resolved source word rather than a
fake numerical lookup. It leaves five string lookups delegated to the
existing required provider. No replacement audio system or synthetic
resource decoder is introduced.

## Tests and verification

`verify_original_attribute_lookup.py` pins **seven complete original
code regions totaling 283 original instructions**, checks
**43 independently selected opcode anchors**, **14 original direct
call edges**, source tree child/key offsets, missing-word constant,
value-offset extraction, exact float/word loads and
`EmptyStrings32` literal. The verifier passes on the private retail
ELF with the listed SHA-256.

`Getchu3OriginalAttributeLookupTests` checks a multi-level decoded
key tree, exact and absent keys, original 0x7FFFFFFF fallback,
raw integer values, NaN payload and negative zero bit preservation,
bounds rejection, the audio volume-scale integration and 1,000
deterministic original float reads.

Local g++ **C++20 optimized, NDEBUG Release, ASan+UBSan, and UBSan**
compilation/tests passed against the recovered implementation and an
identical-API local copy of the existing committed audio descriptor
header. This was NOT a full repository CMake or Windows build.

## Remaining original-data dependencies

The original serialized map/tree population and source record-file
ownership are still unknown. The test constructs an **explicit native
decoded index** to exercise the real source lookup algorithm; it
does not claim to have loaded an attribute tree from the retail disc.
The original string-object path and live original audio file binding
are not complete. The separately required genuine navrange source
resource allocation has still not been identified.

The original game is **not native-Windows playable**. No GitHub Actions
were manually triggered; no PS2 runtime emulation was added.
