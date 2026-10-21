# Ape Escape 3 USA — original text-to-attribute tokenization

This pass recovers **the source-text lexical boundary** used before the
original game's attribute tree is populated. It does not substitute an
invented attribute decoder or create a replacement game engine.

Original owner-private SCUS_975.01 retail executable SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
No copyright-protected game binary/data is committed.

## Verified original source call chain

The original initialization entry `0x2BB03C` stores the source pointer
in the original global structure at `0x7406A8`, then executes these
three subroutines **in this order**:

1. `0x2BA218`: clear/reset the old original parser allocations and
   map state.
2. `0x2BA3AC`: scan the source text with ASCII line feed (0x0A) as
   the physical line separator. The original also has additional
   empty-line/record filtering that has not been completely recovered.
3. `0x2BA584`: parse each line's field structure. At `0x2BA5F4`,
   it sets A2 to **0x3A**, the ':' delimiter, and at `0x2BA5FC`
   it calls `0x1F28C4`. Another call to the same delimiter helper
   occurs at `0x2BA78C`. Later code prepares attribute records and
   uses original key insertion/overwrite at `0x2BA8F0..0x2BA9C0`.

## Recovered native splitter

Original `0x1F28C4..0x1F2A54` is a shared game string-token
helper used elsewhere in the executable, not a general-purpose
replacement parser.

The source loop scans the string sequentially. A separator commits
the current token **even if empty**. At EOF, a pending token is
committed **only if nonempty**. This produces, for example:

| Original text | Native tokens |
|---|---|
| `a:b` | `a`, `b` |
| `a::b` | `a`, `(empty)`, `b` |
| `a:b:` | `a`, `b` |
| `:` | one empty token |
| empty input | no tokens |

`native/include/ae3/original_attribute_text_tokens.hpp` adds native
`originalSplitDelimited_1f28c4`,
`originalAttributePhysicalLineSegments_2ba3ac`,
`originalAttributeSplitColon_2ba5f4`, and a source-level
`originalAttributeTokenizeSource_2bb03c` coordinator.

Important distinction: the coordinator produces **lexical pieces
only**. It intentionally keeps empty physical lines instead of
guessing the retail stage parser's complete record filtering,
validation, numeric conversion, and key/value memory layout.
It does **not** populate the original attribute tree. The existing
source-derived `originalBindParsed_2ba9c0` can accept decoded
entries once those remaining original steps have been recovered.

## Source verification and native tests

`native/tools/verify_original_attribute_text_tokens.py` checks
**four complete original code regions totaling 448 words**,
**23 exact opcode anchors**, **nine source JAL edges**, original
branch targets, the literal 0x0A newline separator, 0x3A colon
separator, and original init → line parsing → colon parsing order.
Passed against the exact owner-provided retail USA executable.

`Getchu3OriginalAttributeTextTokensTests` verifies empty and
repeated separators, no forced trailing empty token, CRLF as raw
input, embedded NUL bytes, colon field records, and **25,000
randomized cases**. Optimized strict C++20, Release NDEBUG, UBSan
and ASan+UBSan passed locally. The local test file and all three
committed source/tool blobs were matched by Git SHA-1 to the
tested local file bytes.

## Source-derived fourth-field numeric conversion

The original parser at `0x2BA6BC` checks for at least **four**
colon-separated fields. At `0x2BA6F0` it calls original
`0x428C48`, with the fourth token pointer loaded from stack +0x0C
in the delay slot. That wrapper invokes original base-10
`0x42E970` and sign-extends its low 32-bit result. The caller
stores the integer at original allocated record offset `+0x20`
(`0x2BA6F8`).

`originalAttributeFourthInteger_2ba6f0` now provides a
source-derived native conversion for ordinary in-range decimal
inputs and returns no value when the parsed row has fewer than
four fields. It uses the host C decimal conversion routine and
preserves the original low-word sign interpretation. Numerical
overflow, locale differences and unusual ASCII inputs are not
yet established to be exact retail equivalents.

The verifier was extended with the complete 10-instruction
`0x428C48` wrapper and 20 instructions around its caller:
**six pinned source regions / 478 instruction words, 28 exact
opcode anchors and 11 call edges**. Additional native cases
cover signed negative values, prefix conversion, absent fourth
fields and the full signed 32-bit endpoints. Optimized C++20,
NDEBUG Release, UBSan and ASan+UBSan passed locally, and all
three uploaded blobs match the corresponding locally tested
files byte-for-byte by Git blob hash.

## Correction: original newline preprocessing is not generic splitting

Further ELF recovery has replaced the earlier generic LF splitter
with the original 0x2BA3AC byte scanner. It inserts a source line
only when a LF byte is encountered, never emits an incomplete
trailing line, and contains the original two-stage `sizest` then
`b:` substring termination logic. These markers and the state
branches were verified at original 0x6F8E08/0x6F8E10 and in
0x2BA488/0x2BA50C/0x2BA53C.

See `ORIGINAL_ATTRIBUTE_LINE_SCANNER.md` for the full
source-backed scanner, verified instruction addresses, and native
text-to-original-attribute-tree integration. The earlier generic
`originalSplitDelimited_1f28c4` remains valid for colon
tokenization; it is not used as a stand-in for the LF scanner.

## Next source dependencies

The deeper `0x2BA584` source field schema, source-object
allocation, original `0x428C48` text-to-number conversion, and
the source bytes originally passed to `0x2BB03C` still need
recovery. Real audio configuration and stage `navrange` allocations
are not yet obtained; they are distinct pipelines. Full Windows
game CMake build and actual game boot not executed.

**This is not a playable native Windows game yet.**
No GitHub Actions are manually invoked.
