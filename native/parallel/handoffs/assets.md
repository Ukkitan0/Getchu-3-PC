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
