# Original Ape Escape 3 — real NavGraph loader and retail ISO assets

**Source:** owner-private USA `SCUS_975.01` hash
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No copyrighted original executable, extracted navigation file or archive is committed.

## What was recovered from the executable

Tracing all direct references to original `0x21D46C` found **one**:
`JAL 0x21D46C` at **0x4F0B14**, inside original `0x4F0A5C`.
The executable's original debug strings identify the subsystem as
`void NavGraph::load(const void*)` and `NavGraph::naviList`.

Original `0x4F0FBC` compares the game strings at:

| Original executable VA | Original name | Native slot |
| --- | --- | --- |
| `0x6E7470` | `nav_monkey.nav` | 0 |
| `0x6E7480` | `nav_enemy.nav` | 1 |
| `0x6E7490` | `nav_extra0.nav` | 2 |

The direct third comparator's result is discarded. The code falls
through to **slot 2** for any string that matches neither monkey nor
enemy; this behavior is faithfully retained rather than assuming a
strict three-string whitelist. The destination pointer slots begin
at original global **0x647B80** (three four-byte pointers).

At `0x4F0A5C`, the original loader enumerates nav resources via
the game's iterator methods around `0x26D120`/`0x26D214`, gets a name
at resource +0 and payload pointer at resource +8, looks up the slot,
and calls `0x21D46C` on its graph with that payload. This
correctly identifies the previously reconstructed geometry routines as
**NavGraph construction and navigation queries**, not an unrelated
generic spatial subsystem.

## Actual real-game archive investigation

The original USA ISO contains `DATA.BIN` with a **VFI** header. Its
actual file table has **3,994 files**, including **168**
`navigation.pck.sz` packages. Each of those packages was read from the
actual retail ISO in this analysis, decompressed with the game's SZ
format (32-bit little-endian output size plus **raw DEFLATE**, not
zlib-wrapped DEFLATE), and parsed as original PCK.

Actual results: **168/168 successful packages**, containing
**168 `nav_monkey.nav`** members and
**122 `nav_enemy.nav`** members, a total of **290 navigation files** and
**19,377 source navigation-record headers**. The first package holds
8,720 bytes of `nav_monkey.nav` with 174 records and 2,048 bytes
of `nav_enemy.nav` with 30 records. Its extracted original
source files were examined privately, not added to GitHub.

For the first navigation package, native optimized C++20 decoding was
independently tested against the actual original ISO. It produced
a 10,864-byte PCK and the exact two real member sizes/counts.

## Native C++ integration

`native/src/original_navgraph_loader.cpp` contains:

- `originalNavGraphSlot_4f0fbc`: reconstructed original slot selection.
- `originalNavInflateSZ`: host zlib **raw** DEFLATE decoder. Input
  is from the **original** VFI archive; output size is validated.
  Uses original end-of-stream behavior and permits the original
  trailing source bytes.
- `originalNavReadPackage`: parses retail PCK with **absolute**
  name offsets and returns actual original `.nav` source member bytes.
- `originalNavLoadGraph_4f0a5c`: sends real original bytes,
  their caller-provided original source allocation VA, and original
  graph owner VA to the already reconstructed original
  `0x21D46C` NavGraph construction code.

The project already has a native VFI/ISO `ae3::Archive` reader;
this code reuses it rather than building a duplicate VFI filesystem.

To inspect your own ISO without starting the game:
```
Getchu3NavGraphProbe.exe "C:\\games\\Ape Escape 3 (USA).iso"
Getchu3NavGraphProbe.exe "C:\\games\\Ape Escape 3 (USA).iso" "navigation.pck.sz"
```

The optional path filter prints per-resource original navigation
record counts. The default prints only total package/member/record
counts; it does **not** write game assets to disk.

CMake links the native zlib system library if available, otherwise
fetches/builds host-native zlib 1.3.1. The new source and test
`Getchu3OriginalNavGraphTests` are registered.

## Authenticity audits, limitations

`native/tools/verify_original_navgraph_routes.py` SHA-pins the
**original ELF**, verifies 2 whole code windows covering **79
instructions**, 16 independently checked instruction sites, five
JAL call destinations, and the actual embedded original function/file
names. Original assets remain private.

The new native test uses *fixture* SZ/PCK/nav data to test decoding,
absolute PCK name offsets, source nav pointers, source-stage loading,
bounds and rejection. The actual retail survey was performed
separately on the real owner-supplied ISO.

**Not yet complete:** original stage/navigation resource lifetime,
real source allocation VA assignment, original object/entity update
methods, movement path following, collision effects, stage display,
full Windows game boot or playability. This is a native asset loading
and source reconstruction milestone, **not** a running game build.

No GitHub Actions/workflows were triggered.
