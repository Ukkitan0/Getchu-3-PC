# Original Game Resources & Asset Formats — specialist handoff

**Branch:** `parallel/assets`  
**Integration target:** `native/original-elf-engine-recovery-20261020`  
**Base:** `d483436416c14927851853f01c6ad66fe4cfbbf2`

## 2026-10-21/22 — ISO/VFI to retail typed-PCK resource ownership checkpoint

**Largest unresolved stage blocker identified:** the authentic stage-selection and resource-owner lifetime chain is not complete. Existing components separately read ISO/VFI, PCK, EXDB, TIM2, and the loaded stage's spatial descriptors. The source routine that selects each stage's VFI entries, obtains the stage allocation and supplies the real allocation VA to `originalStageBuildSourceSpatial_21d46c` has not been recovered end to end.

### Specialist commits (all on `parallel/assets`)

- `c029f9823469cef64f55fd079e58848192baacd4` — `native/include/ae3/original_resource_package.hpp`. Stable, noncopyable/nonmovable original-PCK byte owner; validated absolute name/type string offsets and 0x10-entry table; zero-copy resource payload/name views; source index and exact PCK payload offset; original type-filter boundary behavior; content-signature identification for I3D_BIN/I3D_I3M/I3D_I3C/TIM2/EDB without substitutions. The parser *explicitly applies the previously verified woods_c/exdb.pck table layout*; it does not assert compatibility with all other PCK variants.
- `776162f2daad5bf1d8cd47d082b38aefd554bbd8` — `native/include/ae3/original_resource_vfi_mount.hpp`. Native `Archive` -> original VFI lookup -> original PCK byte allocation -> typed package owner. The caller passes the **real** original VFI path. The returned `std::unique_ptr<OriginalPckResourcePackage>` retains resource spans even after the `Archive`/ISO stream closes. Direct authentic raw-file reads are also provided. There is no guessed stage-name resolution.
- `2c3ce3f122657448df616dd130d1d9673ffa1224` — `native/tests/original_resource_package_test.cpp`. Synthetic structural regression using the documented original EXDB PCK table offsets and source names, seven malformed-table/string/payload rejection cases, strict name/type dispatch, original five payload signatures, and source-view ownership checks. No commercial bytes.

### Provenance (existing retail audits, *not re-run against owner files in this session*)

Original USA `SCUS_975.01` SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

Prior source-backed original resource calls:
- `0x26D24C`: iterates original typed PCK resource entries; `0x26D2B8..0x26D2E0` permits exact type or a following ASCII space/NUL, NOT arbitrary prefix.
- `0x26D138`: copies 16-byte name/type/data/length record to a source resource entry vector.
- `0x26D120`, `0x26D214`: entry count/index.
- `0x35FCBC`, `0x35FD20`: audio requests type `exdb` and exact name `bgm_desc.exdb` (ELF literal VAs `0x7140A0`, `0x7140A8`).
- `0x1BB63C` and `0x2BB03C`: original descriptor / attribute pipeline consumers.
- `0x21D46C`: stage spatial loader, stage source count +0x24, six relative offsets +0x38..+0x4C, 0x1E descriptors, 0x70 native spatial records. The upstream source allocation and stage-selection path is not yet recovered.
- `0x36EDAC`: unresolved real spatial/grid allocator.

Original owner-private `vfi/debug/us/stage/woods_c/exdb.pck` SHA-256 from pre-existing retail audit:
`185db8cfd710760d9c715e0d1cce8ac4ecf6b18201673ce00bc6815078ae4615`.
Observed package size 5728 (`0x1660`); PCK header `PCK\0` +0; 16-byte item table +0x40, 2 entries; `monkeyprofile.exdb` type `exdb` at payload +0x60, size +0x1400; `MainCameraParam.exdb` type `exdb` at +0x1460, size +0x200. Prior verification: `native/tools/verify_original_exdb_retail.py` and `native/tests/original_exdb_retail_fixture_test.cpp`.

**Important discrepancy:** `native/src/pck_archive.cpp` resolves some PCK names as pool-relative offsets and historically accepted pointers into names or onto NUL terminators. The source-verified EXDB PCK table view uses package-relative (absolute within package) offsets. Do not silently replace either interpretation without differential tests on the private `bg.pck` and `woods_c/exdb.pck`; this pass installs a separately labeled, strictly validated EXDB-layout reader instead.

### Actually performed local checks

Environment: Debian, `g++ (Debian 14.2.0-19) 14.2.0`; no private USA ISO, original ELF or extracted game resources were mounted in this working container.

Passed:
```bash
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  -I native/include native/tests/original_resource_package_test.cpp -o resource-package-test
./resource-package-test
# ORIGINAL_RESOURCE_PACKAGE_TEST_PASSED members=2 malformed=7 signatures=5

c++ -std=c++20 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I native/include native/tests/original_resource_package_test.cpp -o resource-package-asan
ASAN_OPTIONS=detect_leaks=1 ./resource-package-asan
# ORIGINAL_RESOURCE_PACKAGE_TEST_PASSED members=2 malformed=7 signatures=5
```
The mount header also passed `-std=c++20 -Wall -Wextra -Wpedantic -Werror` standalone compile-only inspection against the exact `Archive` declaration. **Complete native CMake/CTest, Win32 compile/link, original-ISO roundtrip, and actual game boot are not verified.** No GitHub Actions were triggered.

### Interface, lifetime, and integrator request

```cpp
ae3::OriginalResourceVfiMount mounted(ownedUsaIsoOrDataBin);
std::unique_ptr<ae3::OriginalPckResourcePackage> package =
    mounted.openPck(actualOriginalVfiPckPath);
auto entries = package->entries();
auto selectedIndices = package->selectType_26d24c("exdb");
const auto& resource = package->require("MainCameraParam.exdb", "exdb");
std::span<const std::byte> originalPayload = resource.payload;
```

Do **not** use `originalPayload` after destroying its owner `package`; this is an explicitly borrowed view. The package itself holds all PCK original bytes and cannot be moved. Consumers requiring independent lifetimes must copy or keep `package` alive.

Integration owner only: add the new `native/tests/original_resource_package_test.cpp` as a C++20 test target with include path `native/include`; no new .cpp/library object or additional link dependencies required for the header-only owner. Include `original_resource_vfi_mount.hpp` in stage bootstrap to use existing `Archive` object logic. A follow-up integration test should link `native/src/archive.cpp` and inspect an owner-private ISO / DATA.BIN; hand the package member's source bytes into the *existing* EXDB and spatial consumers only after confirming the original stage entry name. Never edit CMake or another workstream's files from this branch.

### Remaining original-game blockers

1. Verify the new package table and VFI mount on the **actual USA ISO**, and run both existing EXDB differential checks with SHA-256-pinned real `woods_c/exdb.pck`.
2. Recover the retail stage resource registration / allocation / filename selection call chain before passing stage bytes into `originalStageBuildSourceSpatial_21d46c`. This code cannot infer the correct stage-data source by scanning for magic alone.
3. Recover all stage spatial-action dependents, `0x36EDAC` allocator and dependency ownership, and identify stage render/model/texture/animation/audio tables with actual private source offsets.
4. Validate variant PCK string-pointer encodings against separately supplied real `bg.pck`, not the synthetic-only test; reconcile unified parser behavior only after differential proof.
5. Connect consumer lifetimes with graphics, Lua, gameplay and Windows Runtime teams through integration owner. No claimed native playable build.

**No substitute resources, copyrighted ISO/ELF/PCK/Lua bytes, or GitHub Actions changes are committed.**


## Follow-up: native decoding of original VFI SZ resources

Existing owner-audited extraction code (`native/tools/extract_retail_iso.py`, `native/tools/extract_original_stage.py`) establishes the VFI `*.sz` file format: little-endian uint32 expanded byte count at **+0x00**, followed by raw RFC1951 DEFLATE from **+0x04** (`inflateInit2(-MAX_WBITS)`). The native VFI reader itself previously returned compressed source data. This is a separate material blocker to consuming original archived resources directly. **The original retail executable's DEFLATE decoder function VA is still unidentified.**

### Additional commits
- `05edbc68f859900865013f1b6078f6d50c0896cc`: `native/include/ae3/original_resource_sz.hpp`, original SZ reader signature and bounded expanded-size constant (512 MiB).
- `22577197233c7dacb6f1429d3e84fae7d734d80e`: `native/src/original_resource_sz.cpp`, streaming native raw-DEFLATE reader; declared-size and per-call-budget checks, incomplete-stream/trailing-byte rejection, output bound checks, zlib initialization/destruction under RAII.
- `2cd07dc2fc1fa4aa2482f370d4daea84189547dc`: `native/tests/original_resource_sz_test.cpp`, original-format synthetic regression at nine sizes including zero and 524,288 bytes, randomized/high-compression cases, truncation, length mismatch, trailing data, budget cases, and a complete generated **SZ -> PCK -> source-name/type resource -> original I3D signature** chain.
- `fc1e061ef20d9de61aed0323d6a17c4a0d512c5a`: `native/include/ae3/original_resource_vfi_mount.hpp` update. `readOriginalFile` now uses the actual VFI entry filename extension to expand SZ when present; `openPck` can therefore read explicitly named compressed original PCK members, while `readStoredVfiMember` returns byte-exact stored data for checksum comparison. No guessed stage names or substituted assets.

### Actual local test results

GCC 14.2.0, Debian. Both commands completed with successful output:
```bash
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  -I native/include native/src/original_resource_sz.cpp \
  native/tests/original_resource_sz_test.cpp -lz -o resource-sz-test
./resource-sz-test
# ORIGINAL_RESOURCE_SZ_TEST_PASSED sizes=9 guards=length,truncation,trailing,budget

c++ -std=c++20 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I native/include native/src/original_resource_sz.cpp \
  native/tests/original_resource_sz_test.cpp -lz -o resource-sz-asan
ASAN_OPTIONS=detect_leaks=1 ./resource-sz-asan
# ORIGINAL_RESOURCE_SZ_TEST_PASSED sizes=9 guards=length,truncation,trailing,budget
```
The revised mount header also passed strict C++20 compile-only testing with the exact existing `Archive` class declaration. **These are original-format synthetic tests, not a fresh test against owner-private retail files, full CMake/CTest, Win32 link, or game boot.**

**Integrator-only build request:** `native/CMakeLists.txt` already links `Getchu3Assets` against the existing zlib target. Add `src/original_resource_sz.cpp` to the `Getchu3Assets` sources, and add an `original_resource_sz_test.cpp` C++20 executable/test linked against zlib and the SZ implementation. The mounted-resource code requires the new `ae3::originalDecodeSz` linker symbol. No new external runtime library is required beyond the existing zlib target.

**Remaining evidence:** feed a SHA-256-checked owner-private USA original `*.sz` resource to both the new native decoder and the existing `expand_original` Python verifier; compare exact bytes and decoded PCK metadata. Continue identifying original stage archive filenames, authentic runtime allocation/base-VA provenance, dynamic action dependencies and required geometry, animation and audio descriptor resources. A playable retail Windows game is not yet verified.


### Optional owner-private differential PCK regression

Additional commit `002c4391ea7928b83ba18097f9d5b2dd5e31967b`: `native/tests/original_resource_package_retail_test.cpp` reads the **owner's own** original `vfi/debug/us/stage/woods_c/exdb.pck`; checks the documented 0x1660-byte source length, both exact name/type/offset/size records, EDB payload signatures, original 0x26D24C `exdb` type filter and correct absence of `bgm_desc.exdb`. It commits NO resource bytes. The new test was compiled with GCC 14.2.0 using `-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`. Its real-data execution remains **unverified** because the private file is not available in this session (the nonexistent-path check produced the expected error).

Integrator: optionally configure this test only if `AE3_RETAIL_WOODS_C_EXDB_PCK` points to the private original file; use the **existing** `native/tools/verify_original_exdb_retail.py` first to assert the pinned PCK SHA-256 `185db8cfd710760d9c715e0d1cce8ac4ecf6b18201673ce00bc6815078ae4615` and USA ELF SHA-256. Do not install, upload or commit either private file. The native differential test is not evidence of a fresh original-file run until the integrator/owner actually performs it.


## Continuation — retail navigation SZ compatibility and genuine stage resource inventory

### Evidence and new source recovery
Review of existing original navigation source audits (native/docs/ORIGINAL_NAVGRAPH_AND_RETAIL_ASSETS.md and original_navgraph_loader.cpp) confirms: retail NavGraph::load = 0x4F0A5C; direct call to original 0x21D46C at 0x4F0B14. Original resource dispatch is 0x26D120 / 0x26D214 / 0x26D24C. Original nav member names at ELF VAs 0x6E7470, 0x6E7480 and 0x6E7490 select nav_monkey.nav, nav_enemy.nav, and nav_extra0.nav.

Earlier private-USA-ISO research, NOT re-run in this session, counted 3,994 original VFI files, 168 navigation.pck.sz packages, 290 genuine navigation PCK members and 19,377 real three-vertex polygon descriptors. The prior originalNavInflateSZ loader allows trailing raw source bytes; its regression includes four trailer bytes. The assets-owned initial SZ reader was stricter and rejected any trailer. We now provide both strict source-integrity policy and original-compatible permissive mode. No unsupported guess about trailer bytes is substituted for the decompressed payload.

Original navigation PCK and real woods_c/exdb.pck both use package-absolute PCK string offsets. The older generic pck_archive.cpp uses pool-relative offsets. This incompatibility has NOT been silently overwritten; real bg.pck differential testing remains necessary. Original stage directory prefix debug/us/stage/<stage>/ is also evidenced by the existing genuine area.luc load path and the documented private woods_c extraction.

### Committed changes on parallel/assets
- c8bff2b4fa1344c2d6c6809410686a00daa8f805: original_resource_sz.hpp supports OriginalSzTrailerPolicy reject/permit; default strict.
- 88281e6922b832c622f403b7885a037512e8e712: original_resource_sz.cpp respects chosen trailing-source policy.
- 1fb67144408b0b4826af97a36e2a40c85edcde35: original_resource_vfi_mount.hpp uses original-compatible policy for indexed .sz members.
- be910d7d1013b1053552779791c21602e38b34b1: SZ tests cover both policies.
- 2a338aec7a1e115a922f4afb6f83c042ba755f9b: new native/include/ae3/original_resource_stage_catalog.hpp inventories only genuine VFI stage resource names, preserving original relative DATA.BIN offsets, compressed source names, stage ownership, original enumeration order, and exact source lookups. Rejects duplicate case-folded names and unsafe identifiers.
- ce5570a8904b6c7953f75c4292730d607cbc2fc6: native/tests/original_resource_stage_catalog_test.cpp tests two synthetic-stage directory inventories, six stage source members, valid source offsets, source SZ naming, six invalid/traversal/duplicate cases.
- da91925eb3743e675839d8d0459642eb7dd99ee8: new native/src/original_resource_stage_probe.cpp inventories actual private-owner USA ISO/DATA.BIN stages, original VFI members and offsets, optional --inspect-pck typed resource details and source signatures. It writes no original game bytes.
- ecd085b02ecbd0411ac224e1dbf1c82578177403: SZ regression adds a synthetic but original-format navigation SZ -> PCK absolute string name -> nav_monkey.nav type nav -> original header +0x24 count path, permitting a four-byte trailer, without touching navigation-owned files.

### Actual local checks
Host: Debian, GCC 14.2.0 and Clang 17.0.0. These standalone commands were run and passed with C++20 -O2 -Wall -Wextra -Wpedantic -Werror:
- Build tests/original_resource_package_test.cpp with native/include; output ORIGINAL_RESOURCE_PACKAGE_TEST_PASSED members=2 malformed=7 signatures=5.
- Build tests/original_resource_sz_test.cpp with src/original_resource_sz.cpp and -lz; output ORIGINAL_RESOURCE_SZ_TEST_PASSED sizes=9 guards=length,truncation,trailing,budget. Tests include trailing-compatible original-nav resource chain.
- Build tests/original_resource_stage_catalog_test.cpp with native/include; output ORIGINAL_RESOURCE_STAGE_CATALOG_TEST_PASSED stages=2 members=6 malicious=6.
- Syntax-only compile of src/original_resource_stage_probe.cpp with native/include passed.
- Equivalent resource-package/SZ/catalog tests passed under Clang 17.0.0 strict optimized builds.
- AddressSanitizer + UndefinedBehaviorSanitizer with -O1, leak checks enabled: SZ test passed; stage catalog test passed.

These are locally executed synthetic-format tests, not proof of retail original-file compatibility, a complete Windows build or original gameplay. Neither the original USA ISO nor the original executable is available for new tests in this session. No GitHub Actions manually triggered.

### Integrator-only CMake and validation request
Already requested: add src/original_resource_sz.cpp to Getchu3Assets, which already links native zlib. Add target Getchu3OriginalStageResourceProbe from src/original_resource_stage_probe.cpp linked to Getchu3Assets. Add tests for new tests/original_resource_stage_catalog_test.cpp and original_resource_sz_test.cpp with C++20, native/include, plus zlib linkage for SZ (directly or through Getchu3Assets). Optional tests/original_resource_package_retail_test.cpp requires owner-private woods_c/exdb.pck and prior pinned SHA-256 validation.

Once compiled, private-owner sample invocation:
  Getchu3OriginalStageResourceProbe.exe "C:\AE3_PRIVATE\Ape Escape 3 (USA).iso"
  Getchu3OriginalStageResourceProbe.exe "C:\AE3_PRIVATE\Ape Escape 3 (USA).iso" woods_c --inspect-pck

Compare native SZ and PCK outputs byte-for-byte with existing originalNavReadPackage and Python owner-local extractor. Report actual archived filenames, real source SHA-256s, and any PCK variant rejected by strict parsing rather than fabricating an alternative layout.

### Genuine stage blockers left
Original package load-order/ownership call chain and stage transitions; identification of all background/model/texture/animation/audio dependencies on real source bytes; per-stage resource lifetime; real source VA allocation and 0x36EDAC graph/grid allocator; integration of original input/Lua/gameplay/graphics and Windows boot. Stage VFI inventory is not itself an original runtime stage-loader or a playable game.

Only assets-owned source/tests and native/parallel/handoffs/assets.md were modified on parallel/assets. No shared CMake or other categories changed.


### Stage-name authenticity correction (same pass)

Additional commits:
- e73d1b5c7b0f6c2deffb6cd0ba137c3f9859a9a9 — original_resource_stage_catalog.hpp now validates a stage folder as an actual source VFI path component, rather than imposing the existing Win32 launcher application's unrelated 70-character alphanumeric/underscore identifier restriction. This prevents silently excluding genuine source archive folders containing other printable names. Traversal, slash/backslash, control characters and unsafe separators remain rejected.
- ff708ff3fb7cadcaeedbd467f8deed6c79cc8d5a — stage catalog regression now includes a printable non-launcher-style source stage folder. The updated strict optimized GCC 14.2.0 C++20 test passed: ORIGINAL_RESOURCE_STAGE_CATALOG_TEST_PASSED stages=3 members=7 malicious=6. The original retail ISO was not mounted so whether a stage named this way actually exists is NOT asserted; this regression only protects source-fidelity of the filename rules.

The earlier handoff test summary (stages=2 members=6) refers to the earlier commit, not the updated test.

Final local verification after source-stage-name correction: updated stage-catalog test (three synthetic stage folders, seven source members, six rejected malformed lookups) passed GCC 14.2.0 strict -O2, Clang 17.0.0 strict -O2, and GCC AddressSanitizer+UndefinedBehaviorSanitizer with ASAN_OPTIONS=detect_leaks=1. Native stage probe remained syntax-only compiled under GCC -Werror; complete binary linkage and original ISO tests still require integrator and private data.


## Continuation — source-backed stage-scoped resource ownership and original folder-chain tests

**Starting specialist head:** `d22511c2f49e806f747907b64b62a8c2f3749330`.

### Actual ownership/data-flow problem addressed

The earlier ISO/VFI->SZ->PCK package pipeline decoded authentic source bytes but left package and EXDB lifetime management to callers. Borrowed EXDB attribute record spans and original navigation/model payloads must remain valid while their original package allocation is retained, across independent loads. There was no per-stage original-asset owner that could make this boundary explicit.

The **new native ownership boundary does not claim to implement retail stage transition logic**, nor fabricate an EE original pointer VA. It consumes actual, explicitly named VFI assets and releases host-native resources when its stage scope is destroyed. The integration owner must respect that lifetime in graphics, Lua, navigation and gameplay.

### New commits (all source/tests only on `parallel/assets`)
- `5ca2ab94bf2ba5db5a218ee1e86827f502d43c8b` — new `native/include/ae3/original_resource_stage_scope.hpp`. `OriginalStageResourceScope` owns ISO/VFI mount, original stage catalog, lazy immutable original file payloads, PCK allocations, and native EXDB documents in lifetime-safe order. The scope cannot copy/move (borrowed spans remain stable). Exact `file`, `package`, `requireMember`, `filterMembers_26d24c`, `exdb` interfaces reject nonexistent source paths/names/types. Resource maps use original VFI path as cache key, preserving stable pointers across subsequent loads and ASCII-case alternate lookups. Failed original EXDB parsing does not publish a partially decoded document. `decodedExdbCount()` tracks committed documents only.
- `9c0ba0a9558299b4057ac4124302d2977dcf4a69` — new `native/tests/original_resource_stage_scope_test.cpp`. Builds original-format *synthetic* DATA.BIN and ISO source fixture, three VFI stage members (raw `area.luc`, compressed `navigation.pck.sz` with optional nav trailer, original PCK `exdb.pck`), validates stage ownership reuse, actual typed nav payload offset/count, stable borrowed pointers and absent-resource rejection. Intentionally malformed generated EXDB verifies failure leaves no cached decoded document. This is NOT a proof of retail EXDB compatibility.
- `c5e56e6f57a05924fa4c4249b6a558e078e62e24` — new `native/tests/original_resource_stage_scope_retail_test.cpp` (opt-in **private USA ISO/DATA.BIN only**). Checks documented original `woods_c/exdb.pck`, both genuine PCK resource offset/size pairs, original type selection and source magic, then calls the already recovered `0x2BB03C/0x2BB0F4` camera/monkey attribute chain directly from the source PCK allocation: original camera name, area, `lkdown_ang` 30.0, record count/stride; original four monkey profile records and stride. Tests document reuse/stability across an optional separately indexed `navigation.pck.sz`. Retail SHA values and expected offsets derive from PREVIOUS owner-private original audits; these source values were not reproduced/embedded as commercial payload. Actual owner-file execution still pending.
- `dd129b7418e6a08c3594d93ab6111a4592ddb618` — new `native/tests/original_resource_vfi_folder_owner_test.cpp`. Builds VFI's **actual parent-linked folder-table structure** (parent fields are 16-bit table-relative offsets; zero is root sentinel; files have parent IDs). Validates real Archive reconstruction of `debug/us/stage/woods_c/area.luc` and `exdb.pck` from separate `debug`, `us`, `stage`, `woods_c` parent-linked records. Chains decoded VFI paths through the new native stage scope into original-format PCK payload bytes. Rejects invalid parent IDs and cyclic folder ancestry. No retail payload bytes.

### Original code provenance / field offsets (existing evidence, not new ELF access)
- Original USA executable SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- Original resource type dispatch `0x26D24C`; `0x26D2B8..0x26D2E0` type boundary check; `0x26D120`, `0x26D214` vector count/index.
- NavGraph stage load `0x4F0A5C` calls original source-spatial builder at `0x4F0B14 -> 0x21D46C`. `.nav` loaded source descriptor count at +0x24.
- Retail attribute decoding `0x2BB03C`, index `0x2BB0F4`, source string getter `0x2BB3BC`, float getter `0x2BB1D0`.
- Previously SHA-verified private `woods_c/exdb.pck`: original PCK table offset `0x40`, 2 items of 16 bytes; `monkeyprofile.exdb` at payload `0x60` with `0x1400` bytes; `MainCameraParam.exdb` at `0x1460` with `0x200` bytes; PCK SHA-256 `185db8cfd710760d9c715e0d1cce8ac4ecf6b18201673ce00bc6815078ae4615`.
- The archive parser's original VFI source-parent offsets and native layout are inherited from `native/src/archive.cpp`; original retail ELF function VA for this ISO/VFI loader has **not yet been established**. This pass does not invent one.

### Local checks genuinely performed, with qualification
We built an *equivalent local mirror* of the new stage-scope API against copied native archive/SZ source behavior and a **stub for the original EXDB parser**, because the full privately owned retail assets and complete repository toolchain are not present in this session. Strict GCC 14.2.0 C++20 `-O2 -Wall -Wextra -Wpedantic -Werror` stage-owner syntax check passed; fixture exercise with generated DATA.BIN and ISO passed real host file IO, VFI path, SZ raw-DEFLATE, PCK type/name/payload lookup and source-lifetime pointer stability. GCC AddressSanitizer + UndefinedBehaviorSanitizer (`-g -O1 -fsanitize=address,undefined`) passed that fixture. The **EXDB error-path** in this local mirror was a deliberately throwing stub; actual `OriginalExdbDecodedDocument::decode` still requires CMake integration and owner-private execution.

Additional independently generated *nested-folder VFI* fixture passed the same local C++ source-path reader:
- Valid four-level parent-chain `debug/us/stage/woods_c` -> Lua and EXDB PCK source: **PASS**.
- File referring to absent folder table ID: rejected **PASS**.
- Cyclic parent chain: rejected **PASS**.

The existing optimized native resource-package, SZ and stage catalog tests were rerun locally successfully in this session, reporting respectively:
`ORIGINAL_RESOURCE_PACKAGE_TEST_PASSED`,
`ORIGINAL_RESOURCE_SZ_TEST_PASSED`,
`ORIGINAL_RESOURCE_STAGE_CATALOG_TEST_PASSED`.
Complete **new committed C++ test source** was not fully built against the original full repo in this disconnected environment; Win32 linker/game boot and new retail asset validation are unverified. Do not equate the successful local mirror checks to a full integration build.

### Integration-owner-only CMake recipe and native entry points
Previously requested: add `src/original_resource_sz.cpp` to `Getchu3Assets` (existing zlib target is already linked). New stage scope is header-only and requires existing `archive.cpp`, original attribute/EXDB inline decoders, plus the SZ implementation.

Add focused test executables linked to `Getchu3Assets`:
- `native/tests/original_resource_stage_scope_test.cpp`
- `native/tests/original_resource_vfi_folder_owner_test.cpp`
- Optional private USA `native/tests/original_resource_stage_scope_retail_test.cpp` only if `AE3_RETAIL_USA_ISO` is configured; run with that ISO path and independently verify ELF and PCK hashes as existing tools describe.

Integration example, caller must retain the scope as long as any returned span/view/document is used:
```cpp
ae3::OriginalStageResourceScope stage(ownedUsaIso, "woods_c");
auto bytes = stage.file("area.luc");           // borrowed original Lua source
const auto& pkg = stage.package("exdb.pck"); // original, independently owned PCK
const auto& camera = stage.exdb("exdb.pck", "MainCameraParam.exdb");
const auto& navPkg = stage.package("navigation.pck.sz");
// navPkg.originalPackageBytes() can feed the EXISTING original navigation
// team bridge; source runtime allocation VA must be supplied separately.
```
Do not put inferred 32-bit retail VAs into this owner. Do not import/render original geometry in this parser team; graphics owns rendering and original nav team owns stage registration and path selection.

### Next essential source recovery / missing proof
The authentic retail engine's stage bundle selection, resource registration, dependency ordering and teardown call chain is STILL UNKNOWN. Recover its original executable addresses using owner-private USA ELF data. Confirm the exact native new scope with actual original ISO/VFI and the pinned woods_c retail EXDB package, determine live original NavGraph and actor allocation VAs, and identify dependencies required to initialize an authentic stage: source I3D_BIN meshes/material references, TIM2, I3D_I3M, I3D_I3C, Lua 5.0 bytecode, EXDB attributes, nav sources, audio descriptors and game tables. Separate raw parsing from graphics and gameplay execution. No Windows game boot or playable result claimed. No GitHub Actions triggered, no copyrighted content committed, no other team's files/global CMake modified.


## Connected source-format correctness pass — original NavGraph action and selector bounds

**Additional assets-owned commits:**
- `487336411e85132bac0d1c71a83055226c91f099`: `native/src/original_stage_spatial_loader.cpp`.
- `05be3e4cc03b321e6c26ce05b627f83cff3552e0`: `native/tests/original_stage_spatial_loader_test.cpp`.

**Reason / original evidence:** Recovered source constructor `0x21D46C` uses a `0x1E`-byte per-record descriptor, with original action type array beginning +`0x0A`, paired argument array beginning +`0x10`, polygon count at +`0x16`, and vertex indices at +`0x18`. These offsets are already source-attributed and the per-record stride/polygon count were previously SHA-verified against original USA ELF (`native/reference/original_stage_spatial_loader.json`, `native/tools/verify_original_stage_spatial_loader.py`). Exactly three two-byte action type entries and three two-byte arguments fit without overlapping other descriptor fields. Previously a malformed source `actionCount > 3` could cause the native parser to read unrelated polygon data as action types/arguments. The loader now fails with a bounds exception rather than fabricating action metadata. The optional selector index at descriptor +0x00 points to a `0x20`-stride source table via header offset +`0x48`; native bounds checking now requires the **entire 0x20-byte referenced record**, rather than only its first byte.

**Regression additions:** Verify all three action pairs are preserved, count four rejects instead of spilling into vertex fields, a real 0x20-byte indexed extra record resolves to its original 32-bit source VA, and a selector with only one byte left in the source allocation fails. Existing geometry-source, point-count, source owner and missing-vertex checks remain intact. No actor or nav algorithm was added or altered.

**Verification status:** The exact modified existing spatial loader + spatial regression require the coupled VU0/nav library source tree to build. This disconnected local container did **not** run the full new spatial regression after these edits; compile, CTest and original-retail differential checks are still requested from the integrator. The slot constraint is deduced from the already verified descriptor field layout; a new private retail survey of the action-count distribution was **not performed**. Do not interpret the guard as proof all retail gameplay stage action branches are recovered. No changes to global CMake or other workstream files.
