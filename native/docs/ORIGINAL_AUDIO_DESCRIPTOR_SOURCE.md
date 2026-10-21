# Ape Escape 3 USA — original audio descriptor initialization 0x1BB9A8

Owner-private commercial game ELF: USA `SCUS_975.01`,
SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No game executable or assets are committed.

## Important correction to earlier recovery

The previously traced `0x1BB558 -> 0x1BB458 -> 0x1BB9A8 ->
0x1BB3EC -> 0x36D610` chain was a generic 176-byte
record/bucket producer, but its resource category was unknown.

The original ELF's literal data at **0x6D64B0..0x6D6510**
proves that **0x1BB9A8 is AUDIO descriptor population**.
This is not an established `navrange` asset producer. In
particular, absence of a direct JAL to the registered navrange
callback is explained by the independent callback system, not
by evidence that this audio producer creates navrange assets.
Previous native bucket algorithms can still be correct while
resource-category claims about this producer were premature.

## Original descriptor data and exact slot offsets

| Retail property key | Retail default literal | Destination |
|---|---|---|
| `name` | `noname` | +0x00..+0x1F |
| `midi` | `M01.mid` | +0x20..+0x3F |
| `reverb` | `system` | +0x40..+0x5F |
| `vb` | `M01.bd` | +0x60..+0x7F |
| `vh` | `M01.hd` | +0x80..+0x9F |
| `volume_scale` | IEEE754 1.0f | +0xA0 (single precision) |

The five original string-like fields occupy **32-byte object slots**,
NOT necessarily 32-character strings. Their source construction
calls `0x35AB88`, the original attribute lookups call `0x2BB3BC`,
and each output is copied by two 128-bit SQ instructions into the
documented record slot. The float uses `0x2BB1D0` and SWC1.
The original caller `0x1BB458` uses a **0xB0 stride** to populate
each array entry; `0x1BB3EC` hashes the entry's leading 32-byte
native string object into the generic resource bucket table.

## Native source-level translation

`native/include/ae3/original_audio_descriptor.hpp` implements:
- The five original literal property/default pairs and original slot offsets.
- Native `originalAudioDescriptorFields_1bb9a8` querying
  them in the exact original order before `volume_scale`.
- Required `OriginalAudioDescriptorSource` callbacks for the
  still-unrecovered original serialized attribute lookup functions,
  rather than inventing a file parser or guessed sound-file content.
- A host-native typed `OriginalAudioDescriptorValues` representation,
  distinct from the original memory layout. `std::string` is NOT
  serialized into 32-byte retail slots.

This is a recovered original **audio data initialization** boundary,
not PS2 emulation, a substitute sound engine or a fabricated level.

## Verification

`native/tools/verify_original_audio_descriptor.py` checks
the private ELF SHA, SHA-256 of the **complete 92-instruction**
`0x1BB9A8..0x1BBB14` region, five original string constructor calls,
five lookup calls, all ten 128-bit SQ destination stores, the volume
float literal and SWC1, the original 0xB0 array stride, and the
caller JAL from `0x1BB458`. The audit passed against the actual
original executable.

`Getchu3OriginalAudioDescriptorTests` covers all retail defaults,
the original property lookup order, source-provided replacement
values, negative-zero and NaN float-bit preservation, and 1,000
deterministic calls. Compiled and passed locally with strict
optimized C++20 (including NDEBUG), UndefinedBehaviorSanitizer,
and AddressSanitizer+UBSan using the ACTUAL header under test.
A full Windows game-library build and an actual serialized
sound-configuration differential test were NOT performed.

## Open dependencies

The serialized attribute/object parser behind `0x2BB3BC` and
`0x2BB1D0` remains an external native recovery dependency.
The original stage navrange payload allocation and its actual
data-file linkage remain unidentified. The complete ISO is not
available in this working directory. Engine actor updates,
graphics, audio playback backend, scene transitions and Windows
boot still require substantial original-code recovery.

**The complete game is not playable.** No GitHub Actions were
manually triggered.
