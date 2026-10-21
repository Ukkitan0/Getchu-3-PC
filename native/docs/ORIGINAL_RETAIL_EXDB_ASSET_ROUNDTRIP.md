# Ape Escape 3 USA — first real retail EXDB asset round-trip

**Private source evidence:** Original USA retail executable `SCUS_975.01`
(SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`)
and **owner-provided original** `vfi/debug/us/stage/woods_c/exdb.pck`
(SHA-256 `185db8cfd710760d9c715e0d1cce8ac4ecf6b18201673ce00bc6815078ae4615`).
**Neither retail asset is checked into GitHub or embedded in a test.**

## Major correction validated against actual game data

The prior native source function
`originalAttributeHeaderFields_2ba6a4_2ba6f0` read
colon field **0** as the header child-field count. That was wrong:
the original `0x2BA6A8` opcode (`8D840008`) reads **field 2**.
For an actual header, `stn:MainCameraParam:18:1`,
that means **18 source child fields** and **one record**. The
previous implementation produced zero child fields on real data
because decimal conversion of `stn` yields zero. The corrected
function reads token 2 and token 3; the prior synthetic regression
fixtures were updated accordingly.

This was detected by an actual game-data test, not guessed from
the game's theoretical format. The private ELF audit now explicitly
pins the third-field load.

## Actual retail source package

The original 5,728-byte `woods_c/exdb.pck` contains a
`PCK\0` header, a 16-byte-per-entry metadata table at 0x40,
and two authentic original resource entries:

| Original name | Type | Package offset | Byte size |
|---|---|---:|---:|
| `monkeyprofile.exdb` | `exdb` | 0x60 | 0x1400 |
| `MainCameraParam.exdb` | `exdb` | 0x1460 | 0x200 |

Each package table item contains four little-endian 32-bit
fields: relative filename offset, relative type-string offset,
payload offset, and payload length. These bytes were **directly
observed in the retail package**. This host PCK view is a bounded
file-format reader; it is **not** a claim that the original PCK
loader's complete machine code is decompiled.

Both payloads contain `EDB\n` source records. The previously
recovered 0x2BB03C/0x2BA3AC/0x2BA584 code now consumes the
**real original EXDB data**:

* **MainCameraParam:** 18 key fields, 1 data record, stride
  160, record byte offset 352. The `name` source field is
  `camparam_default`, `area` is `woods_c`, and
  `lkdown_ang` is **30.0f**. The `distance` value uses
  original `0x7FFFFFFF` missing-field semantics.
* **MonkeyProfile:** 59 key fields, 4 data records, stride
  1024, record byte offset 1024. The regression reads all
  four source record ranges and a real source `name` field.

The decoded tree, original C-string getter, original numeric
getter, source count/stride accessor and host-bounded source
allocation are connected with **the same original bytes**.

## The retail sound-system loading call chain

The retail executable performs a type selection using the literal
`exdb` at 0x7140A0, then iterates a 16-byte-per-entry resource
array using the original accessors:

* `0x26D120` — record count from (end-begin) / 16.
* `0x26D214` — indexed resource entry.
* `0x35FD20` — compare entry name with
  literal `bgm_desc.exdb` at 0x7140A8.
* `0x35FD5C` — read source entry +0x08 payload pointer.
* `0x35FD64` (with JAL delay slot `0x35FD68`) —
  call original audio-config consumer `0x1BB63C` with
  source entry +0x0C payload length.
* `0x1BB66C` — call `0x2BB03C` to parse the original
  attributes before creating the original 176-byte audio
  configuration record pool.

This **explains** how a real original resource is supplied to
the parser rather than treating the source bytes as arbitrary
synthetic text. The **woods_c** package does **not** contain
`bgm_desc.exdb`, so the source name-filter for the BGM-specific
code correctly returns no entries for this particular stage file.
It is not a sound-file fixture; its EXDB entries verify the
same attribute parser used by original audio configuration.

## Native implementation and verified tests

`native/include/ae3/original_exdb_package.hpp` provides:

* A bounds-checked view of original PCK metadata and EXDB payload
  slices, with source bytes and file names remaining borrowed.
* Original sound-system type/name filtering based on the two
  strings directly recovered from the original ELF.
* `OriginalExdbDecodedDocument`, which owns the native decoded
  red-black tree and uses the existing original parser, original
  source record index and source getters on a retained original
  payload span. It validates the count/stride/data allocation
  before exposing source records.

`original_exdb_retail_fixture_test.cpp` is a REAL GAME DATA
regression. It accepts a path to the user's privately supplied
original `exdb.pck` as its only argument. The fixture is not
committed or reproduced in the test.

`verify_original_exdb_retail.py` checks both original SHA-256
values, **131 whole original ELF instruction words in five source
regions**, **25 independent original opcode anchors**, **eight JAL
edges**, the retail `exdb`/`bgm_desc.exdb` literals and the
genuine package offsets and record values. It passed locally on
both original files.

After correcting the header index, the four existing native parser
regression suites and the new real-retail EXDB test passed GCC
optimized Release and Clang compilation. Original parser and
real EXDB test cases also passed AddressSanitizer+UBSan.
This pass did not run the entire Windows game/library build.

To run the real-retail fixture CTest, configure CMake with the
path to your OWN extracted asset:
`-DAE3_RETAIL_WOODS_C_EXDB_PCK=/path/to/woods_c/exdb.pck`.
When unset, the real-asset test executable is built but the test
is not registered, so normal CI does not require or download
copyrighted game files.

## Remaining blockers

This is the first successful original asset-data round trip in
this parser chain. The original PCK *game loader routine*, stage
archive asset registry, live navigation resource allocation,
actor updates, renderer and native Windows boot are not
reconstructed. The user-supplied ZIP contains only one stage
PCK, not the complete disc's EXDB inventory.

**Ape Escape 3 is not playable as a native Windows application yet.**
No emulation backend, placeholder gameplay or GitHub Actions
workflow was added.
