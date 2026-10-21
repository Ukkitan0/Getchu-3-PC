# Original Ape Escape 3 USA — 0x2BA3AC source line scanner

Retail source: owner-private original USA `SCUS_975.01`, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Original game executable and resource bytes remain private and are not committed.

## Important correction to the earlier attribute parser

The previous `originalAttributePhysicalLineSegments_2ba3ac` was
implemented as a generic `split(text, '\n')`. Disassembly of the
actual original **0x2BA3AC..0x2BA580** reveals additional behavior:

- Reads the byte pointer at source+0x00 and exactly the count at
  source+0x08. In the original 0x2BA424/0x2BA428 pair, only byte
  0x0A commits the accumulated line to the original string vector.
- Non-newline bytes are appended through original `0x627A78`.
  A final fragment with no newline is **not** inserted when the
  original byte-count loop finishes.
- On a committed line, the original `0x627C68` substring finder
  searches for literal `sizest` at original ELF address 0x6F8E08.
  A match sets a persistent flag at 0x2BA490.
- Once that flag is set, a **later** committed line without
  `sizest` is searched for the literal `b:` at 0x6F8E10.
  If `b:` occurs, the original function exits through 0x2BA4C0.
  That terminating line has already been appended, and later
  input bytes are not processed.
- The `sizest` and `b:` matches are substring searches, not
  exact-line equality checks. An earlier `b:` before `sizest`
  does not stop scanning.

The search uses `0x627C68`, which returns 0xFFFFFFFF for
not-found. The original complements that value with NOR and branches
at 0x2BA488, 0x2BA50C and 0x2BA53C. Both literal marker strings were
read directly from the SHA-pinned retail executable; they are not
invented parser delimiters.

## New native recovery and composition

`original_attribute_text_tokens.hpp` now provides
`originalAttributeScanLines_2ba3ac` with explicit committed lines,
marker-state and termination status, and consumed byte count.
The original `originalAttributePhysicalLineSegments_2ba3ac` API is
retained and delegates to that native recovery, so existing callers
pick up the corrected retail behavior.

`original_attribute_loaded_text.hpp` provides
`originalAttributeBindLoadedText_2bb03c`, a source-level
composition of the original 0x2BB03C chain:

```
already-loaded source bytes
   -> 0x2BA3AC newline scanner and source early-stop
   -> 0x1F28C4 colon tokenization
   -> 0x2BA584 header, child fields and trailing owner fields
   -> 0x2BA9C0 decoded-key bind into the recovered RBT
   -> existing original 0x2BB3BC/0x2BB1D0 getters
```

The host validation path parses before clearing the current native
tree, preserving the old tree on malformed source. That is an
intentional **exception-safety improvement**, not a claim of precise
PS2 failure-state parity with 0x2BA218's earlier cleanup step.

The link between original tree node +0x14 and child-record +0x08
remains flattened in the native decoded tree as documented in
`ORIGINAL_ATTRIBUTE_RECORD_PARSER.md`. No fake asset file decoder
or MIPS interpreter is created.

## Verified original source evidence

`verify_original_attribute_line_scanner.py` pins the exact original
ELF SHA and three entire source regions:

- 0x2BA3AC..0x2BA584 — all 118 line-scanner instruction words.
- 0x627C68..0x627CF8 — all 36 original string-find words.
- 0x2BB03C..0x2BB084 — all 18 source dispatcher words.

Across those **172 instruction words**, it checks **21 exact opcode
anchors, 15 direct JAL edges, nine branch destinations**, source
length/character offsets, actual C-string literals and npos. The
verifier passed against the owner-private retail ELF.

`Getchu3OriginalAttributeLineScanTests` covers EOF without newline,
empty lines, CRLF, raw embedded nulls, early stop markers including
ordering and partial matches, original consumed-byte count, and
**10,000 deterministic scanner cases**. A fully connected synthetic
text fixture also populates the original native red-black tree,
recovers owner fields and resolves original audio strings and volume.

Locally GCC C++20 optimized Release, Clang, UBSan and ASan+UBSan
passed; the earlier token/header/trailer C++ test suites also passed
with this corrected header. Every committed source, test, and audit
blob was compared to its local tested file using Git blob SHA-1.
The complete Windows game build is **not tested**.

## Remaining game-port blockers

A complete original disc image/stage resource allocation has not been
loaded through this code. The original serializer's external input
lifetime, exact full 32-bit guest pointer layout and additional parser
state are still incomplete. The real game's native actor update,
graphics, audio playback and startup are not yet connected.
**The original game is still not natively playable on Windows.**
No GitHub Actions were manually started.
