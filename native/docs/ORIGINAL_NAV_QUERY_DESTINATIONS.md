# Original Ape Escape 3 USA — NavGraph destination selection and output modes

This native C++20 recovery continues the retail NavGraph query pipeline:
radius-limited polygon selection (`0x220AA4`), adjacency preparation
(`0x220C00`), shortest-path search (`0x220D5C`), and route backtracking
(`0x220994`). It adds the **original terminal-selection logic** invoked
by the query entry at `0x2208A8..0x220948`.

Private retail source verification: original user-owned US `SCUS_975.01`
SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`
and original `woods_c/navigation.pck` SHA-256
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.
Neither copyrighted input is committed to GitHub.

## Recovered original terminal modes

At `0x220F60`, the game visits the query's selected 56-byte search
records and calls the original `0x21B468` polygon centroid routine
for each. The VU0 instruction chain measures XYZ distance from that
centroid to the query position. The best starts at original
`FLT_MAX` (retail literal `0x7F7FFFFF`, VA 0x6E47EC), and
`C.OLT.S` at `0x220FE8` updates the chosen **search-node index**
only for a strictly smaller distance. Equal distances therefore
retain the first source-order node. No nodes returns -1.

`0x221030` follows the same source adjacency enumeration but
initializes the best to `-FLT_MAX` (original `NEG.S` at
`0x22106C`, source literal VA 0x6E47F0); `C.OLT.S` at
`0x2210BC` updates only for strictly greater distance. It
returns the *farthest* selected search-node index, again keeping the
first node for ties.

Original caller `0x2208A8` selects output behavior using mode $9 and a separate index $8:

| Source output mode | Source function | Result |
|---|---|---|
| 0 | `0x221030` | Farthest polygon centroid |
| 1 | `0x220F60` | Nearest polygon centroid |
| Other | direct index | Separate original $8 search-node index; -1 means no route |

Unlike modes 0 and 1, every other mode forwards independent $8 to `0x220994`; the numeric mode itself is never used as an index. The resulting index is processed by the original `0x220994` path
backtracker, recovered in the preceding pass.

## Native integration

`native/include/ae3/original_nav_query_destinations.hpp` provides
`originalNavNearestQueryNode_220f60`,
`originalNavFarthestQueryNode_221030`,
`originalNavDestinationForMode_2208a8`, and
`originalNavDestinationPath_2208a8`. These call the **previously
recovered VU0 distance function**, use decoded search-node centroids,
and preserve the original strict tie-handling. The native public
NavGraph interface exposes the new functions.

These represent the original algorithm using typed host IDs, **not**
binary-compatible PS2 pointers or a replacement AI navigation engine.

## Original executable and genuine source-data verification

`verify_original_nav_query_destinations.py` SHA-pins three exact
retail code windows totaling **133 original instruction words**,
with **23 exact opcode anchors, four JAL edges, six branch targets**,
both source FLT_MAX literals, strict compare instructions, and the
original mode caller. The retail `woods_c` navigation package's
39 monkey and 20 enemy polygons are validated as distinct source
inputs.

`original_nav_query_destinations_test.cpp` tests zero candidates,
strict tie retention, original mode mapping, a provided search index,
invalid-mode rejection and correct path output. On both genuine
`woods_c` graphs it exercises three distinct query points, compares
nearest/farthest selection against an independently calculated
double-precision distance oracle, and verifies routes from the
starting polygon to the selected final polygon.

Local GCC Release and Clang Release, ASan+UBSan, UBSan and retail
ELF verification passed. The source/test/verifier Git blobs match
the locally tested files byte-for-byte. The native test links a
minimal isolated implementation of existing recovered VU0 routines;
**the complete repository Windows game has not been built or booted**.

The synthetic test runs normally under CTest; the optional real
source test uses the user-provided original file at the
`AE3_RETAIL_WOODS_C_NAVIGATION_PCK` CMake variable.

## Remaining original game code

The NavGraph query can now choose and reconstruct a route natively,
but does not move a real game character. Live owner-list binding for recovered `0x21F89C` membership eligibility, game object / actor ownership,
original actor AI/movement consumers, stage renderer, Lua scripts,
and native Windows game boot are not yet reconstructed.

**This commit is not a playable Windows port of Ape Escape 3.**
No emulator, fake gameplay layer, or GitHub Actions run was introduced.
