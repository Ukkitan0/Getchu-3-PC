# Native integration ledger â€” 2026-10-22

Integration target: `native/original-elf-engine-recovery-20261020`.
Baseline: `d483436416c14927851853f01c6ad66fe4cfbbf2`. Working branch: `codex/integration-20261022`.

## Decisions

Selected cluster: native bootstrap, VFI/SZ/PCK ownership and registration, Lua bridge, and rejection of unresolved actor positions. Preserve specialist history; review each selected branch separately before merging. Other workstreams are blocked from this pass pending code and dependency review, not rejected or discarded.

No changed-path overlaps occur between the eight current specialist deltas. This does not establish API compatibility. Main and all specialist refs remain unchanged.

## Verified outcome of this pass

- Accepted histories: runtime `f91297e`, assets `a63a70f`, Lua `13f9c38`, gameplay `092cb98`, each merged separately on the new Codex branch. Their implementations are compiled from the actual Git checkout with MSVC; acceptance does not certify every recovered routine's complete original semantics.
- Pending histories: navigation `76b916d`, VU0 `a34ca34`, graphics `62bdd9b`, input/audio `1641205`. Their original branches and work are preserved. No refused/discarded commits.
- New recovery: native `originalManagerInitialize_57bcb8`, original span `0x57BCB8..0x57BF58` inclusive (169 words), SHA-256 `90ced0cff1cd0ad2f9fa260859db156e1dcc6fb4c4c295dfe0d62fe0df6a9e0d`. Complete branch orchestration translated with explicit external calls; general projection `0x21B854` and actor-owner adapter for existing `0x57C7CC` are still unresolved dependencies. No actor/gameplay execution claim.
- Duplicate check: inspected the existing callback dependency and unchanged related helpers at baseline plus all eight current specialist tips. Existing `0x221104`, `0x21B444`, `0x21B458`, `0x57BDB8`, `0x57C98C`, `0x4748AC` are reused, not replaced or independently retranslated. The earlier manager initialization commit `f6745c3` concerns `0x57BA00`, not this new `0x57BCB8` routine.
- New native files: `native/include/ae3/original_manager_initializer.hpp`, `native/src/original_manager_initializer.cpp`, `native/tests/original_manager_initializer_test.cpp`, `native/tools/verify_original_manager_initializer.py`. CMake source and tests are registered by the integrator.
- Authentic checks: original full ELF SHA and complete initializer span, ten JAL targets, ten vector operand/order sites, original literal/method pairs verified. Focused tests and all 15 genuine woods_c type-2 directed actions passed. The same original RNG and weighted triangle implementation already present is reused.
- Complete MSVC x64 Release build succeeds, including Getchu3Native.exe. Latest full suite: 144 registered checks, 143 passed, zero failed, one explicitly blocked/skipped all-corpus execution check. Release assertions are enabled.
- Corpus: all 168 area.luc chunks match private handoff bytes against the genuine ISO via stage-owned registry; 135,820 unchanged Lua opcodes and 805 original functions structurally retargeted. 88 chunks execute top-level on native Lua 5.0; 77 hit the explicitly unavailable lua_param manager and three boss chunks hit unresolved enlarge_se_range. No setupArea calls counted as corpus execution or gameplay. Unexpected errors fail instead of being skipped.
- Actual Win32 boot: window/D3D11/original ISO indexing PASS; six original static constructors execute; first blocker `0x38AA18`, native exit 4. New actor initializer is not yet reached by game startup.
- Baseline failures repaired: std::optional implicit bool test conversion; fallback negative tests reused already-mutated route state and expected incorrect raw float bytes; old synthetic I3C/PCK fixture used pool-relative offsets; assets scope test used decompressed aliases absent from actual VFI; Windows fixture deleted still-open source streams. Original engine algorithms were not changed to satisfy those tests.
- Source API safeguards: strict retail actor/parameter/weapon bindings; reject typed Lua metatables/shared aliases and preserve raw API path. Stage audit no longer invents default spawn or a live manager from descriptors. Source-only fixture hooks are explicitly labeled synthetic.
- Review completed with no remaining important findings. General-route callback receives an immutable copy of incoming register-vector value, not a mutable field alias.

## API and build reconciliation

- No source-path overlaps across the eight specialist deltas; semantic disagreement found in absolute PCK offsets vs old synthetic I3C fixture, repaired in fixture after original five-package differential checks.
- Literal VFI `exdb.pck.sz` is retained. No unsupported `exdb.pck` alias is introduced. Original type filter is applied before duplicate-name lookup (`area.eff`, `area.eft`, `area.luc`).
- Selected assets' new translation unit, all selected new tests, probes, original source verifiers and Lua audit regressions are registered in `native/cmake/ReviewedStageIntegration.cmake` alongside the reviewed runtime manifest. Remaining specialists' newer sources/tests are intentionally pending, not silently integrated.
- Source owners outlive borrowed registry/EXDB/Lua views. Private cache paths are checked; supplying corpus without required ISO fails configure. No proprietary game inputs are staged, packaged or uploaded.
- Source status: initializer source-verified/translated/compiler-tested/original-data-tested; owning scheduler integration and Windows retail gameplay are unverified. Startup remains blocked at constructor #7.

## Per-commit disposition details

### windows-runtime

Tip `f91297e5908a31ae9ecc41c068dcc3bf4ccbcc66`; merge base `d483436416c14927851853f01c6ad66fe4cfbbf2`; 83 commits; 19 changed paths.

Decision: ACCEPTED for cluster validation.

- `974ea123de02f3cc8bde0489e7e4e51070211a58` â€” ACCEPTED for cluster validation. runtime: fail closed when original retail stage lacks genuine engine boot
  Paths: `native/src/app_win32.cpp`
- `45cc04041db789278b79a0a1d882a8b9dec13662` â€” ACCEPTED for cluster validation. runtime: add one-click native Windows CMake build and optional authentic retail diagnostics
  Paths: `native/BUILD_NATIVE_WINDOWS.ps1`
- `fad94b277a949ac559dd4ee6504f92406c0b7764` â€” ACCEPTED for cluster validation. runtime: add one-click native Windows CMake build and optional authentic retail diagnostics
  Paths: `native/BUILD_NATIVE_WINDOWS.bat`
- `b34c5e04aad255f06cd3e58afc6045d1c2510e14` â€” ACCEPTED for cluster validation. parallel: document source changes, static checks and genuine native boot blockers
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `88c892741d7c892663270abe5aa0aa0a233b6426` â€” ACCEPTED for cluster validation. runtime: run full registered native CTest suite by default in one-click build
  Paths: `native/BUILD_NATIVE_WINDOWS.ps1`
- `d25dea5a2538f9e3545a04a5b9455a89d9b84ad1` â€” ACCEPTED for cluster validation. runtime handoff: record default full CTest and remove placeholder source citation
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `cf0ee1d63a26c8d6fa51c9e506d9ee5fefd6a148` â€” ACCEPTED for cluster validation. runtime handoff: record latest one-click default CTest change
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `f705266da1ced6393804b057de711bffa13f2443` â€” ACCEPTED for cluster validation. Add fail-closed native original-game boot checkpoint journal
  Paths: `native/src/windows_runtime_boot_status.hpp`
- `01901067c13faaf016fce86f2cb690f0069279c2` â€” ACCEPTED for cluster validation. Test native game startup checkpoint truthfulness
  Paths: `native/tests/windows_runtime_boot_status_test.cpp`
- `1f87097605cecf8e544bcf7f9a21521949947864` â€” ACCEPTED for cluster validation. Register native Windows retail startup checkpoint test
  Paths: `native/CMakeLists.txt`
- `a45f90bf72745d84ae0847848f1f224a78250ed4` â€” ACCEPTED for cluster validation. Record real retail startup phases and reject failed D3D11/timer initialization
  Paths: `native/src/app_win32.cpp`
- `b7b1e5dc2c0799e2bb1f9f767943fc8660b9959a` â€” ACCEPTED for cluster validation. Keep native retail startup regression assertions enabled in Release
  Paths: `native/tests/windows_runtime_boot_status_test.cpp`
- `115386e25912fa650bde234a8eaf5d7b3fa7b4bc` â€” ACCEPTED for cluster validation. Document tested runtime checkpoints, strict boot integration and remaining original-engine blockers
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `824b916e50f22159724075eb1c364737ea15ebbf` â€” ACCEPTED for cluster validation. Record isolated Release CMake/CTest validation without claiming Windows boot
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `560ea8352796120462573a9430bfad1890c5ae76` â€” ACCEPTED for cluster validation. Stage complete original-engine workstream sources and native regression targets
  Paths: `native/cmake/ParallelIntegration.cmake`
- `92b94b45a8df2836f58cf9423a20175ab7aef360` â€” ACCEPTED for cluster validation. Wire conditional original-engine specialist integration manifest into CMake
  Paths: `native/CMakeLists.txt`
- `47a2c1ddccfb188d2ad836349c9bf9ef3c4c5cdf` â€” ACCEPTED for cluster validation. Preserve source VU0 floating-point order and Win32 pad test linker dependencies
  Paths: `native/cmake/ParallelIntegration.cmake`
- `5822388591bb74951b0ca2d93697e0d29a3a53da` â€” ACCEPTED for cluster validation. Allow unique per-retail-run native startup checkpoint logs
  Paths: `native/src/app_win32.cpp`
- `dfcb6f41902601ec45eca5c40f888daa55cee0e8` â€” ACCEPTED for cluster validation. Record per-attempt Windows retail boot blockers and verify optional USA executable SHA
  Paths: `native/BUILD_NATIVE_WINDOWS.ps1`
- `cafb7ba9b89db8f920d6b530a6febb8a8bab5f09` â€” ACCEPTED for cluster validation. Test complete and partial native specialist source registration in isolated CMake fixtures
  Paths: `native/tools/test_parallel_integration_manifest.py`
- `24c9df74af21473de7428ae2a8c6bc02418d5b73` â€” ACCEPTED for cluster validation. Register native parallel source manifest regression in CTest when Python is available
  Paths: `native/CMakeLists.txt`
- `36c2031a2dd532b54c3a49cf70ed7bbdf72968a2` â€” ACCEPTED for cluster validation. Make CMake manifest regression work with Windows multi-config generators
  Paths: `native/tools/test_parallel_integration_manifest.py`
- `682440cf73b7cd2e49de4400a1e2f789d7727d67` â€” ACCEPTED for cluster validation. Document staged original-engine specialist integration and native boot evidence checks
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `37dfa928a64d017bd0e4ec405e474e2ecf36c405` â€” ACCEPTED for cluster validation. Recover original USA Windows-native startup lifecycle original_windows_bootstrap.hpp
  Paths: `native/include/ae3/original_windows_bootstrap.hpp`
- `4ca51404e9e6100e8eaa64e60c8276bf1e2cb958` â€” ACCEPTED for cluster validation. Recover original USA Windows-native startup lifecycle original_windows_bootstrap.cpp
  Paths: `native/src/original_windows_bootstrap.cpp`
- `6d2a886df06d565a913faa5b082ac45aff76962f` â€” ACCEPTED for cluster validation. Recover original USA Windows-native startup lifecycle original_windows_bootstrap_test.cpp
  Paths: `native/tests/original_windows_bootstrap_test.cpp`
- `b50eca8e7ad5442c7ab1afb41a1207fd25b2f736` â€” ACCEPTED for cluster validation. Compile source-authenticated retail constructor sequence and termination registry in native engine
  Paths: `native/CMakeLists.txt`
- `c36d4adb35fcd761bf4746bea38b7d526000e332` â€” ACCEPTED for cluster validation. Verify genuine USA ELF constructor table and native bootstrap call edges
  Paths: `native/tools/verify_original_windows_bootstrap.py`
- `dd9687879d854c71ba7ab83b5c64420cc71e810f` â€” ACCEPTED for cluster validation. Trace genuine original static constructor bootstrap separately from Lua asset workbench
  Paths: `native/src/app_win32.cpp`
- `2062ec7e208d4e133ff3174df38f6d75aa794d9c` â€” ACCEPTED for cluster validation. Log original USA 0x428AB8 constructor dispatch as separate boot checkpoint
  Paths: `native/src/windows_runtime_boot_status.hpp`
- `2554e265bb0a7d74595987279bf26f420e492704` â€” ACCEPTED for cluster validation. Verify all 143 authentic original constructor addresses via ELF-derived native fingerprint
  Paths: `native/tests/original_windows_bootstrap_test.cpp`
- `c466f8482634f747b09538372a3c7ba323a3b444` â€” ACCEPTED for cluster validation. Add one-click genuine original-constructor boot attempt with ELF audit and process exit evidence
  Paths: `native/BUILD_NATIVE_WINDOWS.ps1`
- `01796c158406539e2eabde6c6385b9ae05331964` â€” ACCEPTED for cluster validation. Port verified original 0x41A390 startup constructor counter semantics into native boot
  Paths: `native/include/ae3/original_windows_bootstrap.hpp`
- `28c2d789d3b02ad4ae0bf0c2aa8d6b59a772e523` â€” ACCEPTED for cluster validation. Port verified original 0x41A390 startup constructor counter semantics into native boot
  Paths: `native/src/original_windows_bootstrap.cpp`
- `a261d901b95fd2c8f4f8195dcfdcf01e13e5e59c` â€” ACCEPTED for cluster validation. Port verified original 0x41A390 startup constructor counter semantics into native boot
  Paths: `native/tests/original_windows_bootstrap_test.cpp`
- `9bc020c728fb7f2470c3e9e1e638cd3f8b7bd729` â€” ACCEPTED for cluster validation. Port verified original 0x41A390 startup constructor counter semantics into native boot
  Paths: `native/src/app_win32.cpp`
- `981e893e99501bbbd5b26d818415fb4bac00d0bf` â€” ACCEPTED for cluster validation. Audit exact USA ELF first constructor and genuine no-op callee before native translation
  Paths: `native/tools/verify_original_windows_bootstrap.py`
- `f5663f74680c80419ca7efb643e31dd3ceff079c` â€” ACCEPTED for cluster validation. Document source-verified USA startup, first ported allocator constructor and remaining authentic boot boundary
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `2baaabb5e48b14ead2ce1d5593191ccf3763d401` â€” ACCEPTED for cluster validation. Reject counterfeit retail boot sources by indexing the authentic native ISO/VFI before constructors
  Paths: `native/src/app_win32.cpp`
- `1ae9e7b202aec277322ec03f70d9388ccdf0dab9` â€” ACCEPTED for cluster validation. Give recovered original global constructor state genuine process lifetime
  Paths: `native/src/app_win32.cpp`
- `200abd6435fa6a1d086756b54dcfc32c794249c0` â€” ACCEPTED for cluster validation. Handoff genuine retail ISO/VFI preflight and original constructor lifetime fix
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `78773ee558be1e77e681c1ce6407f0cffda21b29` â€” ACCEPTED for cluster validation. Opt-in genuine original USA ELF opcode audit as CTest with owner-private source
  Paths: `native/CMakeLists.txt`
- `6f02b6ac0c3537bb2649993a416a854696b43239` â€” ACCEPTED for cluster validation. Clarify authentic startup ordering and owner-private optional ELF CTest evidence
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `915d19600c47e1d7eabf3979364ae0a8a6859633` â€” ACCEPTED for cluster validation. Record local authentic USA source audit and isolated C++ Release CTest 2-of-2 pass
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `839bd5795b1cdb228fbca2f89a74e78354ca457b` â€” ACCEPTED for cluster validation. Recover source-verified 0x417C58 original native numeric globals and dispatch
  Paths: `native/include/ae3/original_windows_bootstrap.hpp`
- `56e2818e966c3e57ba22aa6e741248f6bd1a8ec8` â€” ACCEPTED for cluster validation. Recover source-verified 0x417C58 original native numeric globals and dispatch
  Paths: `native/src/original_windows_bootstrap.cpp`
- `3e16441826d78de1080f94e0b5b10b86a203078b` â€” ACCEPTED for cluster validation. Recover source-verified 0x417C58 original native numeric globals and dispatch
  Paths: `native/tests/original_windows_bootstrap_test.cpp`
- `d5d7cbb6d7176713d01805e06bb86bbd2894eb84` â€” ACCEPTED for cluster validation. Persist original retail second-constructor numeric globals for native boot
  Paths: `native/src/app_win32.cpp`
- `8a83002d0bf5cabcf414a12441c171eb0865dd3e` â€” ACCEPTED for cluster validation. Advance verified original Windows native bootstrap through second constructor 0x417C58
  Paths: `native/src/app_win32.cpp`
- `f3796df0cafa74edb706e065f0ee5d8f52d541b9` â€” ACCEPTED for cluster validation. Audit source opcodes for original second constructor, native sqrt inputs and exact call targets
  Paths: `native/tools/verify_original_windows_bootstrap.py`
- `07ffeddef2febe9c8d0e599b732ed7d7065560c6` â€” ACCEPTED for cluster validation. Record second retail constructor and next unresolved startup target
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `bf31657bec257c1d30459b31577390d5ec6fd93e` â€” ACCEPTED for cluster validation. Assert source-derived native boot advances exactly two real constructors and blocks at 0x3A6CD0
  Paths: `native/tests/original_windows_bootstrap_test.cpp`
- `14e50ac855ae838c80bc5c717c100f6c7f7295a2` â€” ACCEPTED for cluster validation. Fingerprint authentic third-constructor static literal block and instructions in USA ELF audit
  Paths: `native/tools/verify_original_windows_bootstrap.py`
- `0d889411ca7e51fa69f593c3bfc219cfd42ce564` â€” ACCEPTED for cluster validation. Handoff second authentic constructor implementation, 2-of-143 boot boundary and third original source data mapping
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `a5e6bbdfab6bce3eb8ac81e03c0191466bf866be` â€” ACCEPTED for cluster validation. Add source-defined native data boundary for original third static constructor
  Paths: `native/include/ae3/original_windows_static_resources.hpp`
- `190d52a446852e47363d783bbf18fb21662ab399` â€” ACCEPTED for cluster validation. Recover original third static constructor immutable records and native differential test
  Paths: `native/src/original_windows_static_resources.cpp`
- `ccdab9e67ccae567243d162eb70c19b34d660aee` â€” ACCEPTED for cluster validation. Recover original third static constructor immutable records and native differential test
  Paths: `native/tests/original_windows_static_resources_test.cpp`
- `76062c9d6327a36b4f37f71437ecaa176a49d451` â€” ACCEPTED for cluster validation. Integrate original third constructor source and optional owner-private regression into CMake
  Paths: `native/CMakeLists.txt`
- `99ae6752fec81205e10ef90461f9cf460a3a55b6` â€” ACCEPTED for cluster validation. Declare native original USA ELF source-region mapping for third constructor
  Paths: `native/include/ae3/original_windows_static_resources.hpp`
- `ab69f5a139c8505825d2cc98f32dfca93df2bf3b` â€” ACCEPTED for cluster validation. Map authenticated owner-provided USA ELF PT_LOAD for native third constructor
  Paths: `native/src/original_windows_elf_source.cpp`
- `530be9608da7abeff5b594317808bf110e80d067` â€” ACCEPTED for cluster validation. Map authenticated owner-provided USA ELF PT_LOAD for native third constructor
  Paths: `native/tests/original_windows_elf_source_test.cpp`
- `8c5ee5df3bea59837f01e12efb8edf2215d1bf52` â€” ACCEPTED for cluster validation. Build source-backed original ELF virtual-address reader and test with original data
  Paths: `native/CMakeLists.txt`
- `8ef672e5c112d3c83b65ce976cb2a20d27f17224` â€” ACCEPTED for cluster validation. Run genuine third native constructor using owner ELF source VA, never embedded game data
  Paths: `native/src/app_win32.cpp`
- `6f86db0ab9bb32be56733252bce8193922a5c5d2` â€” ACCEPTED for cluster validation. Pass SHA-256-verified private USA ELF to original Windows startup for source-backed constructors
  Paths: `native/BUILD_NATIVE_WINDOWS.ps1`
- `ca8e41ccf83ad492ce26361a32707b3ece8bedbe` â€” ACCEPTED for cluster validation. Test first three authentic native constructors and exact fourth blocker using private ELF
  Paths: `native/tests/original_windows_bootstrap_test.cpp`
- `9a6151933b22800065db82fea62a250cdbe4d7a6` â€” ACCEPTED for cluster validation. Test exact native three-constructor boundary against owner-private original ELF
  Paths: `native/CMakeLists.txt`
- `853a9026645ae7ad4344b923db8e80002c5e5782` â€” ACCEPTED for cluster validation. Declare original 0x39FC78 source-backed 38-qword native state
  Paths: `native/include/ae3/original_windows_static_resources.hpp`
- `6fc01b049089e52cff7806836fc8983a4a7f3a74` â€” ACCEPTED for cluster validation. Translate original 0x39FA10 constructor 38 literal copies without guest emulation
  Paths: `native/src/original_windows_static_resources.cpp`
- `e7012a1feee280509a0c1ff62305408700419187` â€” ACCEPTED for cluster validation. Read original fourth constructor's 608 private literal bytes by ELF32 PT_LOAD VA
  Paths: `native/src/original_windows_elf_source.cpp`
- `811349b2a087f5350abb150f001589b666a9d0a7` â€” ACCEPTED for cluster validation. Differential test source-backed original fourth constructor 38 native qwords
  Paths: `native/tests/original_windows_fourth_ctor_test.cpp`
- `ce4225b3988ec39f7c5c8d639795c76e651f04e8` â€” ACCEPTED for cluster validation. Register original fourth static constructor native tests with optional private ELF source
  Paths: `native/CMakeLists.txt`
- `522b40277c9554a49d4c6e93fb36d7569b27d5f4` â€” ACCEPTED for cluster validation. Dispatch four authentic native static constructors from private ELF source records
  Paths: `native/src/app_win32.cpp`
- `aaceffd62785db639fce4bf3c95bc7dab6fa2e65` â€” ACCEPTED for cluster validation. Validate four source-backed native constructors and fifth original blocker against private ELF
  Paths: `native/tests/original_windows_bootstrap_test.cpp`
- `b93239f9e09ec1576abecff568b2b20adfccd5b1` â€” ACCEPTED for cluster validation. Verify authentic fourth native static ctor opcodes and 38 original source qword records
  Paths: `native/tools/verify_original_windows_bootstrap.py`
- `2746e5c530f46c2b13629194ceaf98e64301f417` â€” ACCEPTED for cluster validation. Reflect verified four-original-constructor progression in private ELF CTest
  Paths: `native/CMakeLists.txt`
- `556b60a1ec9fe6041573fbc7b2adbb0791e29332` â€” ACCEPTED for cluster validation. Recover authentic original startup no-op and two native global record resets
  Paths: `native/include/ae3/original_windows_bootstrap.hpp`
- `d4591db8047f80d3ca26a7e9858e21b03d01dd0c` â€” ACCEPTED for cluster validation. Recover authentic original startup no-op and two native global record resets
  Paths: `native/src/original_windows_bootstrap.cpp`
- `2238c36c9597b6f91a9ddf21ae91faca6d0a709c` â€” ACCEPTED for cluster validation. Run recovered fifth and sixth authentic original static constructors on Win32 boot
  Paths: `native/src/app_win32.cpp`
- `d36d575e7e22e4ac11d3e2a489e8a5bacd2a5b5c` â€” ACCEPTED for cluster validation. Validate original fifth no-op, sixth selective resets and exact seventh boot blocker
  Paths: `native/tests/original_windows_bootstrap_test.cpp`
- `28616929cab217d6eb45be93822c92c47b540d6c` â€” ACCEPTED for cluster validation. Pin exact source opcode evidence for original fifth no-op and sixth native field reset
  Paths: `native/tools/verify_original_windows_bootstrap.py`
- `1763a9c8726e8776a416b2705c5479e259c5a378` â€” ACCEPTED for cluster validation. Pin source-original seventh constructor heap allocator and list-owner dependency
  Paths: `native/tools/verify_original_windows_bootstrap.py`
- `0332ae25e6c75d80a778e02a71eb8043e80c383c` â€” ACCEPTED for cluster validation. Handoff original constructors 3-6 native integration, source evidence, tests and seventh allocator blocker
  Paths: `native/parallel/handoffs/windows-runtime.md`
- `f91297e5908a31ae9ecc41c068dcc3bf4ccbcc66` â€” ACCEPTED for cluster validation. Record isolated native CMake Release 4-of-4 constructor-source tests and limitations
  Paths: `native/parallel/handoffs/windows-runtime.md`

Tests absent from baseline CMake (runtime conditional manifest also needs review): `native/tests/original_windows_bootstrap_test.cpp`, `native/tests/original_windows_elf_source_test.cpp`, `native/tests/original_windows_fourth_ctor_test.cpp`, `native/tests/original_windows_static_resources_test.cpp`, `native/tests/windows_runtime_boot_status_test.cpp`

### gameplay-engine

Tip `092cb984924064e2b1bbdc10c2efa151cfafcfa7`; merge base `d483436416c14927851853f01c6ad66fe4cfbbf2`; 4 commits; 4 changed paths.

Decision: ACCEPTED for cluster validation.

- `165b48f92b545227178aa9e3154a87acd2aaae71` â€” ACCEPTED for cluster validation. gameplay: reject unverified retail actor spawn coordinates instead of fabricating origin
  Paths: `native/src/actor_world.cpp`
- `47851309027cb3c9d35909fd3042b8711cf52e8e` â€” ACCEPTED for cluster validation. gameplay: reject unverified retail actor spawn coordinates instead of fabricating origin
  Paths: `native/include/ae3/actor_world.hpp`
- `17f276a1eb73e369005a437a83405188dbfa44df` â€” ACCEPTED for cluster validation. gameplay: reject unverified retail actor spawn coordinates instead of fabricating origin
  Paths: `native/tests/actor_world_test.cpp`
- `092cb984924064e2b1bbdc10c2efa151cfafcfa7` â€” ACCEPTED for cluster validation. parallel: document source changes, static checks and genuine native boot blockers
  Paths: `native/parallel/handoffs/gameplay-engine.md`

Tests absent from baseline CMake (runtime conditional manifest also needs review): none

### navigation

Tip `76b916d6d50bf9fc145beac1079aa2868a1f2531`; merge base `d483436416c14927851853f01c6ad66fe4cfbbf2`; 18 commits; 20 changed paths.

Decision: BLOCKED: outside selected cluster; needs review.

- `b77a5fd741ab0c1359fa73fc91b5998380dc7e0f` â€” BLOCKED: outside selected cluster; needs review. Connect original stage nav owners, type-1 route costs and actor-route source identities (0x2207D0/0x220F18/0x57BD88)
  Paths: `native/include/ae3/original_nav_stage_owners.hpp`, `native/include/ae3/original_nav_stage_route.hpp`, `native/src/original_nav_stage_route.cpp`, `native/tests/original_nav_stage_route_test.cpp`
- `cbd03b0af81725a421aebf4612e57870d5896295` â€” BLOCKED: outside selected cluster; needs review. Harden stage route identity and add multi-stage source-asset regression + integration handoff (0x21ED30/0x2207D0)
  Paths: `native/include/ae3/original_nav_stage_owners.hpp`, `native/include/ae3/original_nav_stage_route.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_stage_route.cpp`, `native/tests/original_nav_stage_route_test.cpp`
- `8040c9fa40d22c7d222cec8213c03ea6d60dafc4` â€” BLOCKED: outside selected cluster; needs review. Bind original navrange group eligibility to live stage routing and connect type-2 actor targets (0x4E4060/0x21EF3C/0x57BDB8)
  Paths: `native/include/ae3/original_nav_stage_groups.hpp`, `native/include/ae3/original_nav_stage_route.hpp`, `native/src/original_nav_stage_groups.cpp`, `native/src/original_nav_stage_route.cpp`, `native/tests/original_nav_stage_groups_test.cpp`, `native/tests/original_nav_stage_route_test.cpp`
- `01b6c689eda08b838ec64e4a8bb4fad04603bf28` â€” BLOCKED: outside selected cluster; needs review. Correct source route follower/fallback expectations and document navrange stage integration boundaries
  Paths: `native/parallel/handoffs/navigation.md`, `native/tests/original_nav_stage_groups_test.cpp`, `native/tests/original_nav_stage_route_test.cpp`
- `4228074fdb865553fb02c10009c5a8f9f33bacc5` â€” BLOCKED: outside selected cluster; needs review. Guard original navrange-bound routes against mutable group membership and stage rebinds
  Paths: `native/include/ae3/original_nav_stage_groups.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_stage_groups.cpp`, `native/tests/original_nav_stage_groups_test.cpp`
- `387e1fc9564cfe14150fd8f6f583a8af919fdf7c` â€” BLOCKED: outside selected cluster; needs review. Bind original source routes to live actor navigation lifetime, target selection and follower callbacks
  Paths: `native/include/ae3/original_nav_actor_binding.hpp`, `native/include/ae3/original_nav_stage_groups.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_actor_binding.cpp`, `native/tests/original_nav_actor_binding_test.cpp`
- `ff3c8332eaf3f47ee5d4d308435531e5d7c4c426` â€” BLOCKED: outside selected cluster; needs review. Preserve no-route actor semantics and invalidate navrange routes after failed registration
  Paths: `native/parallel/handoffs/navigation.md`, `native/src/original_nav_actor_binding.cpp`, `native/src/original_nav_stage_groups.cpp`, `native/tests/original_nav_actor_binding_test.cpp`
- `f4c334416cf1d1f961733343d970b627ab4b5598` â€” BLOCKED: outside selected cluster; needs review. Connect original 0x2207D0 source routes directly to existing 0x57C7CC stage actor pipeline without VU0 substitutions
  Paths: `native/include/ae3/original_nav_actor_binding.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_actor_stage_bridge.cpp`, `native/tests/original_nav_actor_stage_bridge_test.cpp`
- `622083d971b0c60c1fc0bb90b132b7ad0d88e05a` â€” BLOCKED: outside selected cluster; needs review. Regenerate original actor routes on target changes with safe 0x2207D0 admission and group-key dispatch
  Paths: `native/include/ae3/original_nav_actor_binding.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_actor_binding.cpp`, `native/tests/original_nav_actor_binding_test.cpp`
- `4091c10086dc6c0ff426942dedd4f9bf5a7b4bef` â€” BLOCKED: outside selected cluster; needs review. Fail closed on interrupted original navrange registration to prevent actor routing through partial group ownership
  Paths: `native/include/ae3/original_nav_stage_groups.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_stage_groups.cpp`, `native/tests/original_nav_actor_binding_test.cpp`, `native/tests/original_nav_stage_groups_test.cpp`
- `e6972b7d8f45b300bc7496bf991ba22e735a084a` â€” BLOCKED: outside selected cluster; needs review. Validate original 0x220994 route integrity before actor handoff while preserving 0x221104 source-edge semantics
  Paths: `native/include/ae3/original_nav_path_integrity.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_actor_binding.cpp`, `native/src/original_nav_stage_route.cpp`, `native/tests/original_nav_actor_binding_test.cpp`, `native/tests/original_nav_path_integrity_test.cpp`, `native/tests/original_nav_stage_route_test.cpp`
- `252f8342469f074a40a00eecd34ad22ba87d0739` â€” BLOCKED: outside selected cluster; needs review. Record GCC and Clang sanitizer execution of standalone navigation route-integrity regression
  Paths: `native/parallel/handoffs/navigation.md`
- `4d1d9714cd766965ff276f9904a0e747f6609b40` â€” BLOCKED: outside selected cluster; needs review. Invalidate original 0x2207D0 member-owned routes on 0x21F82C list mutation, erase, clear and disposal
  Paths: `native/include/ae3/original_nav_linked_owner.hpp`, `native/include/ae3/original_nav_stage_route.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_linked_owner.cpp`, `native/src/original_nav_stage_route.cpp`, `native/tests/original_nav_actor_binding_test.cpp`, `native/tests/original_nav_linked_owner_test.cpp`, `native/tests/original_nav_stage_route_test.cpp`
- `6d3e8853c89ec22081506f59f3fb1d2cad15ea44` â€” BLOCKED: outside selected cluster; needs review. Reject untracked raw 0x4E3DAC membership spans at long-lived native actor route boundary
  Paths: `native/include/ae3/original_nav_stage_route.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_actor_binding.cpp`, `native/src/original_nav_stage_route.cpp`, `native/tests/original_nav_actor_binding_test.cpp`, `native/tests/original_nav_stage_route_test.cpp`
- `a84a5c8da3bd177ecc513468f577dcdd0cc1813b` â€” BLOCKED: outside selected cluster; needs review. Document original navigation membership lifetime commits and integration test evidence
  Paths: `native/parallel/handoffs/navigation.md`
- `4cfaec330997eb3ff9eb6e724b43b152e2583d70` â€” BLOCKED: outside selected cluster; needs review. Add SHA-pinned source CFG decoder for original actor controller 0x57BCB8 with offline tests
  Paths: `native/parallel/handoffs/navigation.md`, `native/tools/analyze_original_nav_controller_57bcb8.py`, `native/tools/test_analyze_original_nav_controller_57bcb8.py`
- `8dea744824da0d20e82a71e423e97e9475ed3aa8` â€” BLOCKED: outside selected cluster; needs review. Allow exact original 0x21EF3C eligibility callbacks in live 0x4E4060 navigation group registration
  Paths: `native/include/ae3/original_nav_stage_groups.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_stage_groups.cpp`, `native/tests/original_nav_stage_groups_test.cpp`
- `76b916d6d50bf9fc145beac1079aa2868a1f2531` â€” BLOCKED: outside selected cluster; needs review. Forward original 0x21EF3C navrange source-frame inputs; correct MIPS branch-likely controller CFG decoding
  Paths: `native/include/ae3/original_nav_stage_groups.hpp`, `native/parallel/handoffs/navigation.md`, `native/src/original_nav_stage_groups.cpp`, `native/tests/original_nav_stage_groups_test.cpp`, `native/tools/analyze_original_nav_controller_57bcb8.py`, `native/tools/test_analyze_original_nav_controller_57bcb8.py`

Tests absent from baseline CMake (runtime conditional manifest also needs review): `native/tests/original_nav_actor_binding_test.cpp`, `native/tests/original_nav_actor_stage_bridge_test.cpp`, `native/tests/original_nav_path_integrity_test.cpp`, `native/tests/original_nav_stage_groups_test.cpp`, `native/tests/original_nav_stage_route_test.cpp`

### vu0-physics

Tip `a34ca340a0a566e6d98caa707afccaddaae1b598`; merge base `d483436416c14927851853f01c6ad66fe4cfbbf2`; 37 commits; 47 changed paths.

Decision: BLOCKED: outside selected cluster; needs review.

- `cf94d593f047436d40695fc9d3091c7645ab122a` â€” BLOCKED: outside selected cluster; needs review. Recover original 0x57CA68 native VU0 candidate vectors and packed target integration
  Paths: `native/include/ae3/original_actor_fallback_sampler.hpp`, `native/include/ae3/original_actor_fallback_vu0_native.hpp`, `native/include/ae3/original_actor_navigation_pipeline.hpp`, `native/include/ae3/original_actor_target_dispatch.hpp`, `native/src/original_actor_fallback_sampler.cpp`, `native/src/original_actor_fallback_vu0_native.cpp`, `native/src/original_actor_navigation_pipeline.cpp`, `native/src/original_actor_target_dispatch.cpp`, `native/tests/original_actor_fallback_vu0_native_test.cpp`
- `0be4b445a095c20347e0a6ad9414f9608bdb42cf` â€” BLOCKED: outside selected cluster; needs review. Test recovered 0x57CA68 candidate-to-actor packed target path
  Paths: `native/tests/original_actor_fallback_pipeline_test.cpp`
- `2ae2becd8d53015c5ab69b52c7777ab71590ad68` â€” BLOCKED: outside selected cluster; needs review. Reset route state before connected native VU0 regression
  Paths: `native/tests/original_actor_fallback_pipeline_test.cpp`
- `68b72d599ef03657fdbc6654b481a2bf2d6e10a3` â€” BLOCKED: outside selected cluster; needs review. Audit original fallback VU0 packed registers, masks and native transform instruction sequences
  Paths: `native/tools/verify_original_actor_fallback_vu0_native.py`
- `df51478dd21ef081dd2a3d957195bbef64d81cb7` â€” BLOCKED: outside selected cluster; needs review. Fix original ELF binary-magic assertion in VU0 source auditor
  Paths: `native/tools/verify_original_actor_fallback_vu0_native.py`
- `487e732f133b920aba8f73f8683681958d9826cd` â€” BLOCKED: outside selected cluster; needs review. Preserve source DMTC2 low-doubleword semantics and connect original VU0 R state
  Paths: `native/include/ae3/original_actor_fallback_sampler.hpp`, `native/include/ae3/original_actor_navigation_pipeline.hpp`, `native/include/ae3/original_actor_target_dispatch.hpp`, `native/include/ae3/original_stage_actor_navigation.hpp`, `native/src/original_actor_fallback_sampler.cpp`, `native/src/original_actor_navigation_pipeline.cpp`, `native/src/original_stage_actor_navigation.cpp`, `native/tests/original_actor_fallback_pipeline_test.cpp`
- `c7634f60524f6ea0af6466c90169b1fa355b90e6` â€” BLOCKED: outside selected cluster; needs review. Extend original VU0 fixed-vector regression to W lanes and zeroed lower lanes
  Paths: `native/tests/original_actor_fallback_vu0_native_test.cpp`
- `36edb0e60625a981dacc7f65d0beca19b96d12df` â€” BLOCKED: outside selected cluster; needs review. Handoff genuine 0x57CA68 VU0 recovery, private ELF audits, native tests and integration API
  Paths: `native/parallel/handoffs/vu0-physics.md`
- `ecd5a8b98c10ad3de6450ff056048acb52fd9f28` â€” BLOCKED: outside selected cluster; needs review. Fix source QMTC2/PCPYUD 128-bit actor target handoff and triangle vector lanes
  Paths: `native/include/ae3/original_actor_navigation_pipeline.hpp`, `native/include/ae3/original_actor_random_triangle.hpp`, `native/include/ae3/original_actor_target_dispatch.hpp`, `native/src/original_actor_navigation_pipeline.cpp`, `native/src/original_actor_random_triangle.cpp`, `native/src/original_actor_target_dispatch.cpp`, `native/tests/original_actor_random_triangle_test.cpp`, `native/tests/original_actor_target_dispatch_test.cpp`, `native/tools/verify_original_actor_fallback_vu0_native.py`
- `524b63405abfee165227cebd57fa591f90347c66` â€” BLOCKED: outside selected cluster; needs review. Expose source 0x57C894 PCPYUD projection operand and test all 128-bit lanes
  Paths: `native/include/ae3/original_actor_target_dispatch.hpp`, `native/src/original_actor_navigation_pipeline.cpp`, `native/src/original_actor_target_dispatch.cpp`, `native/tests/original_actor_target_dispatch_test.cpp`
- `fccb9dd272856d9b6937773da2759e72c915130f` â€” BLOCKED: outside selected cluster; needs review. Expose full original 0x57C98C QMFC2 packed target for native actor movement
  Paths: `native/include/ae3/original_actor_random_triangle.hpp`, `native/src/original_actor_navigation_pipeline.cpp`, `native/src/original_actor_random_triangle.cpp`, `native/tests/original_actor_random_triangle_test.cpp`
- `c10d79ad027e8e32d88070fad8087475464d52ed` â€” BLOCKED: outside selected cluster; needs review. Document verified 128-bit QMTC2/PCPYUD correction and update VU0 handoff
  Paths: `native/docs/ORIGINAL_ACTOR_FALLBACK_57CA68.md`, `native/docs/ORIGINAL_ACTOR_RANDOM_TRIANGLE.md`, `native/parallel/handoffs/vu0-physics.md`
- `9921e54713ca46e87ee0b24138677641907619d6` â€” BLOCKED: outside selected cluster; needs review. Verify source 0x57C894/0x57C98C packed vertex and projection operands
  Paths: `native/tools/verify_original_actor_fallback_vu0_native.py`
- `1672842d5c2cd644589d309b06136b79a9f64b70` â€” BLOCKED: outside selected cluster; needs review. Audit all 30 original fallback VU0 operands and masked destination lanes
  Paths: `native/tools/verify_original_actor_fallback_vu0_native.py`
- `0e736112572f29ede6edc10f3c3aaa6c8ec890f0` â€” BLOCKED: outside selected cluster; needs review. Recover distinct original 0x57C814 and 0x57C890 projection A1 packing
  Paths: `native/include/ae3/original_actor_ray_geometry.hpp`, `native/src/original_actor_navigation_pipeline.cpp`, `native/src/original_actor_ray_geometry.cpp`, `native/tests/original_actor_ray_geometry_test.cpp`, `native/tools/verify_original_actor_fallback_vu0_native.py`
- `997460c91001d85a987f72175cfbfe5df7f7c932` â€” BLOCKED: outside selected cluster; needs review. Handoff 30-register VU0 operand audit and source-accurate two-ray actor geometry
  Paths: `native/docs/ORIGINAL_ACTOR_RAY_GEOMETRY.md`, `native/parallel/handoffs/vu0-physics.md`
- `13d2ddbc16ad85717dd11561d33f4fa74f85dbff` â€” BLOCKED: outside selected cluster; needs review. Preserve genuine VU0 R operations in every native fallback path and test shared binding
  Paths: `native/include/ae3/original_actor_navigation_pipeline.hpp`, `native/src/original_actor_navigation_pipeline.cpp`, `native/tests/original_actor_fallback_pipeline_test.cpp`
- `34850c2e335d533c19e03aa2ff290d663f04ac51` â€” BLOCKED: outside selected cluster; needs review. Finalize VU0 R side-effects, isolated native test evidence and branch handoff
  Paths: `native/parallel/handoffs/vu0-physics.md`
- `9a1f815a538186a931122405f2026291f9562e7f` â€” BLOCKED: outside selected cluster; needs review. Test authentic six-candidate VU0 selector against independent source math on 59 original polygons
  Paths: `native/tests/original_actor_fallback_native_selection_test.cpp`
- `f268df88454ce428b43d9b3d4611bd43d8818580` â€” BLOCKED: outside selected cluster; needs review. Handoff source-native six-candidate winner test and precise CMake integration
  Paths: `native/parallel/handoffs/vu0-physics.md`
- `58862811068524a1cd84d0e6165aca98021139b3` â€” BLOCKED: outside selected cluster; needs review. Cover genuine 0x21EE00 epsilon, signed zero, equality and polygon W-lane clipping
  Paths: `native/tests/original_polygon_native_raycast_test.cpp`
- `c6e1568719324f58140d886f49f181d1a42a5407` â€” BLOCKED: outside selected cluster; needs review. Recover genuine actor type-two VU0 target interpolation 0x57BDCCâ€“0x57BED4 with native SQC2 controller writeback
  Paths: `native/include/ae3/original_actor_type2_vu0_target.hpp`, `native/src/original_actor_type2_vu0_target.cpp`, `native/tests/original_actor_type2_vu0_target_test.cpp`, `native/tools/verify_original_actor_type2_vu0_57bdcc.py`
- `8c6942f7915029eae0f63168bae4e3e3511459ef` â€” BLOCKED: outside selected cluster; needs review. Document original 67-instruction type-two actor VU0 movement and integration contract
  Paths: `native/docs/ORIGINAL_ACTOR_TYPE2_VU0_57BDCC.md`
- `f3636311a0a277d889c4ca5fdf6fed3b18245fea` â€” BLOCKED: outside selected cluster; needs review. Test type-two native navigation output through original VU0 interpolation and controller SQC2 write
  Paths: `native/tests/original_actor_type2_vu0_target_test.cpp`
- `2bb98763de8a2c20146d37750a0676b35ef6c3a3` â€” BLOCKED: outside selected cluster; needs review. Handoff recovered original kind-two VU0 movement, source audits and native collision tests
  Paths: `native/parallel/handoffs/vu0-physics.md`
- `08ecabf71fa46614e8dce2cc8a8643916db21a50` â€” BLOCKED: outside selected cluster; needs review. Recover original 0x57BCDC distance gate and connected type-2 actor movement branch
  Paths: `native/include/ae3/original_actor_distance_gate.hpp`, `native/include/ae3/original_actor_type2_movement_bridge.hpp`, `native/src/original_actor_distance_gate.cpp`, `native/src/original_actor_type2_movement_bridge.cpp`, `native/tests/original_actor_distance_gate_test.cpp`, `native/tests/original_actor_type2_movement_bridge_test.cpp`
- `1e47d99d422dcf11a2335e48f7751bdb8f69db41` â€” BLOCKED: outside selected cluster; needs review. Audit genuine 31-instruction actor distance gate 0x57BCDCâ€“0x57BD54
  Paths: `native/tools/verify_original_actor_distance_gate_57bcdc.py`
- `a136c00bf79c5a5285d22a3216798b010031040d` â€” BLOCKED: outside selected cluster; needs review. Test original distance gate continuation into real type-two movement beyond 2.0 threshold
  Paths: `native/tests/original_actor_type2_movement_bridge_test.cpp`
- `4d321df6da72c67532fee1ca0f2bc1922c0f0fe3` â€” BLOCKED: outside selected cluster; needs review. Document authentic actor distance reset/type-2 branch and integration tests
  Paths: `native/docs/ORIGINAL_ACTOR_DISTANCE_GATE_57BCDC.md`, `native/parallel/handoffs/vu0-physics.md`
- `73577196b0187f9f2d0aa7a1f01e7e629c525494` â€” BLOCKED: outside selected cluster; needs review. Recover original actor planar XZ distance and bit-exact controller return branches
  Paths: `native/include/ae3/original_actor_controller_vector_compare.hpp`, `native/include/ae3/original_actor_xz_proximity.hpp`, `native/src/original_actor_controller_vector_compare.cpp`, `native/src/original_actor_xz_proximity.cpp`
- `bd7d4ad7c977892226204d77ce9e553ef961e623` â€” BLOCKED: outside selected cluster; needs review. Test native XZ proximity and bitwise controller compare on authentic stage polygons
  Paths: `native/tests/original_actor_controller_vector_compare_test.cpp`, `native/tests/original_actor_xz_proximity_test.cpp`
- `2c40e8e7a560c57b000e0c312e1d9070d2d826be` â€” BLOCKED: outside selected cluster; needs review. Audit 44 retail actor XZ/packed-compare instructions, original VU0 and MMI registers
  Paths: `native/tools/verify_original_actor_xz_and_compare_57c080.py`
- `09f028004ab73dd6b0b9741a01d9d697c61b0ee4` â€” BLOCKED: outside selected cluster; needs review. Test exact source PCEQW/PPACH/NOR equality against independent packed-word oracle
  Paths: `native/tests/original_actor_controller_vector_compare_test.cpp`
- `c607c632d1c4e1df35464a2aeacb0d432c8904c9` â€” BLOCKED: outside selected cluster; needs review. Normalize XZ original-actor helper header to tested source
  Paths: `native/include/ae3/original_actor_xz_proximity.hpp`
- `55179f15fd13e955380ef62c460265fc3c57e534` â€” BLOCKED: outside selected cluster; needs review. Clarify provenance argument name for original 0x21B854 output
  Paths: `native/include/ae3/original_actor_xz_proximity.hpp`
- `42d88947f319b532bcf6bb5b248e667fdb869c70` â€” BLOCKED: outside selected cluster; needs review. Document original 0x57C080 XZ packing and 0x57C1D4 raw controller return recovery
  Paths: `native/docs/ORIGINAL_ACTOR_XZ_AND_COMPARE_57C080.md`
- `a34ca340a0a566e6d98caa707afccaddaae1b598` â€” BLOCKED: outside selected cluster; needs review. Handoff retail XZ VU0 distance and packed controller return recovery with private original tests
  Paths: `native/parallel/handoffs/vu0-physics.md`

Tests absent from baseline CMake (runtime conditional manifest also needs review): `native/tests/original_actor_controller_vector_compare_test.cpp`, `native/tests/original_actor_distance_gate_test.cpp`, `native/tests/original_actor_fallback_native_selection_test.cpp`, `native/tests/original_actor_fallback_vu0_native_test.cpp`, `native/tests/original_actor_type2_movement_bridge_test.cpp`, `native/tests/original_actor_type2_vu0_target_test.cpp`, `native/tests/original_actor_xz_proximity_test.cpp`

### graphics-animation

Tip `62bdd9b10a648638f3d036bcc8c698ec8a6370c1`; merge base `d483436416c14927851853f01c6ad66fe4cfbbf2`; 101 commits; 59 changed paths.

Decision: BLOCKED: outside selected cluster; needs review.

- `0d52f7a39279a0ff91e4234209d1f354b92b28d4` â€” BLOCKED: outside selected cluster; needs review. graphics: add native original-mesh instance and independent animation render submission
  Paths: `native/include/ae3/original_render_instances.hpp`
- `4543a3fb8f305929d89710dc7e73ad860c60114f` â€” BLOCKED: outside selected cluster; needs review. graphics: add native original-mesh instance and independent animation render submission
  Paths: `native/src/original_render_instances.cpp`
- `c72012f65f68ce7396a66ae99593134e6dbcf1e1` â€” BLOCKED: outside selected cluster; needs review. graphics: add native original-mesh instance and independent animation render submission
  Paths: `native/tests/original_render_instances_test.cpp`
- `54470c47c29dab746c76739d37ffdbdfdf32c1f4` â€” BLOCKED: outside selected cluster; needs review. graphics: preserve authentic exported material colors without fabricated sunlight
  Paths: `native/src/gltf_scene.cpp`
- `021624cbca63b43ac31c1f491309901e18607fe4` â€” BLOCKED: outside selected cluster; needs review. graphics test: assert exported material base color is unmodified
  Paths: `native/tests/gltf_scene_test.cpp`
- `d437cf06af933075977b3c6c44d2e4efefadc876` â€” BLOCKED: outside selected cluster; needs review. graphics: add SHA-gated real USA ELF rendering and camera reference audit
  Paths: `native/tools/audit_original_graphics.py`
- `a91d4c5195241db61967ed28ee43491c80c8b4d3` â€” BLOCKED: outside selected cluster; needs review. graphics: correct SHA-verified ELF magic and string-byte matching
  Paths: `native/tools/audit_original_graphics.py`
- `bc982c6ce8ebbef29afe10a920b6c673f8d9a234` â€” BLOCKED: outside selected cluster; needs review. graphics: recover USA retail 0x280838 early visibility gate and native test
  Paths: `native/include/ae3/original_render_visibility.hpp`
- `7eafa467686f5355de9fff998ca3fd5c59b8b5fc` â€” BLOCKED: outside selected cluster; needs review. graphics: recover USA retail 0x280838 early visibility gate and native test
  Paths: `native/src/original_render_visibility.cpp`
- `440227080d2c84d644ab361787eed75e925bf77f` â€” BLOCKED: outside selected cluster; needs review. graphics: recover USA retail 0x280838 early visibility gate and native test
  Paths: `native/tests/original_render_visibility_test.cpp`
- `03a1eac0bee2a5521bab9f43c9edcef62d636ebc` â€” BLOCKED: outside selected cluster; needs review. graphics: validate exact original MIPS pre-render gate instruction fingerprint
  Paths: `native/tools/audit_original_graphics.py`
- `b8cf7a7ebe93284cb45e59a599a5864607680513` â€” BLOCKED: outside selected cluster; needs review. graphics: document verified USA ELF render gate and native scene pipeline boundaries
  Paths: `native/docs/ORIGINAL_RETAIL_RENDER_GATE_AND_INSTANCING.md`
- `2f79f5abc35301b6a4e47825d99ce606925d5c4d` â€” BLOCKED: outside selected cluster; needs review. graphics: hand off real retail gate, renderer integration, tests, exact blockers
  Paths: `native/parallel/handoffs/graphics-animation.md`
- `1b928ffdbdb85560d060e2f4ecd1fe8d9d7afd97` â€” BLOCKED: outside selected cluster; needs review. graphics: port authentic USA render-selector lifecycle and dispatch route
  Paths: `native/include/ae3/original_render_selector.hpp`
- `f4b29dfb7ad5b765c11a6866fb34d773be91faa2` â€” BLOCKED: outside selected cluster; needs review. graphics: port authentic USA render-selector lifecycle and dispatch route
  Paths: `native/src/original_render_selector.cpp`
- `b836070cb9d25d686b925b460c5829ab729f4957` â€” BLOCKED: outside selected cluster; needs review. graphics: port authentic USA render-selector lifecycle and dispatch route
  Paths: `native/tests/original_render_selector_test.cpp`
- `5e50b5fad016751d445ac4abbd1c26cac71a3790` â€” BLOCKED: outside selected cluster; needs review. graphics: fingerprint original selector set/get/reset and retail draw branch
  Paths: `native/tools/audit_original_graphics.py`
- `bb7841a0ead43e57a9c00592825d3b162c228984` â€” BLOCKED: outside selected cluster; needs review. graphics: document original selector MIPS routines, nested fields and rendering route
  Paths: `native/docs/ORIGINAL_RETAIL_RENDER_GATE_AND_INSTANCING.md`
- `ad7c163b6a45aefd1688253fee8b749f5d424f65` â€” BLOCKED: outside selected cluster; needs review. graphics handoff: source-backed selector lifecycle, tested route API and exact integration
  Paths: `native/parallel/handoffs/graphics-animation.md`
- `49ac85e3205a644c2a0fbdae16f0f7d762af0056` â€” BLOCKED: outside selected cluster; needs review. graphics handoff: verify retail fingerprints and identify VU0 frustum dependency
  Paths: `native/parallel/handoffs/graphics-animation.md`
- `27eaeae50df1b1bff29ffc6602b6edceefd3b4d2` â€” BLOCKED: outside selected cluster; needs review. graphics: recover original primary and secondary retail render pass call ordering
  Paths: `native/include/ae3/original_render_pass.hpp`
- `fe3bb6160127b8adda49fa5fe48cb90b9aa58da4` â€” BLOCKED: outside selected cluster; needs review. graphics: recover original primary and secondary retail render pass call ordering
  Paths: `native/src/original_render_pass.cpp`
- `a06ae6e38d69c0f6b5c8b4e4d134b75cac619e8b` â€” BLOCKED: outside selected cluster; needs review. graphics test: verify original primary secondary render control with compiler sanitizers
  Paths: `native/tests/original_render_pass_test.cpp`
- `cd70a9ddd6543b85a180d69710b4c3c580f022b5` â€” BLOCKED: outside selected cluster; needs review. graphics: preserve original render traversal order through authentic scene instances
  Paths: `native/include/ae3/original_render_pass.hpp`
- `90424193a34e2cc918299e2da53186c2a0948fd8` â€” BLOCKED: outside selected cluster; needs review. graphics: preserve original render traversal order through authentic scene instances
  Paths: `native/src/original_render_pass.cpp`
- `facb8c6b7a408788ee473309bb65e62f507bef10` â€” BLOCKED: outside selected cluster; needs review. graphics: preserve original render traversal order through authentic scene instances
  Paths: `native/include/ae3/original_render_instances.hpp`
- `2c1f3f049838526dce22bbd215b70213c7d2892a` â€” BLOCKED: outside selected cluster; needs review. graphics: assemble original render pass selected geometry in genuine traversal order
  Paths: `native/src/original_render_instances.cpp`
- `27a58d0e351e4361c2f7d2f287330ced5be1c260` â€” BLOCKED: outside selected cluster; needs review. graphics tests: verify retail actor visibility traversal to D3D scene submission
  Paths: `native/tests/original_render_instances_test.cpp`
- `0ed52dc30b72047621c3f460b4ad3a4282bfe678` â€” BLOCKED: outside selected cluster; needs review. graphics tests: verify retail actor visibility traversal to D3D scene submission
  Paths: `native/tests/original_render_pass_test.cpp`
- `e0a63403c14fe73321e6113fd0beb2072cafb3f4` â€” BLOCKED: outside selected cluster; needs review. graphics: audit SHA-verified USA retail primary and secondary draw traversal instruction ranges
  Paths: `native/tools/audit_original_graphics.py`
- `9d4fd19028af9589dd551f913853627baad76fd3` â€” BLOCKED: outside selected cluster; needs review. graphics: source port USA original render owner activation, frame modulo and scheduler
  Paths: `native/include/ae3/original_render_owner_schedule.hpp`
- `0fe7e4c951074c4ffd03dae633117fa905f465ca` â€” BLOCKED: outside selected cluster; needs review. graphics: source port USA original render owner activation, frame modulo and scheduler
  Paths: `native/src/original_render_owner_schedule.cpp`
- `b6a564527ad851655fb858516cb80325d3fb9fce` â€” BLOCKED: outside selected cluster; needs review. graphics: source port USA original render owner activation, frame modulo and scheduler
  Paths: `native/tests/original_render_owner_schedule_test.cpp`
- `3af740d0337e994f217ebec2cbcb5a484e2035ce` â€” BLOCKED: outside selected cluster; needs review. graphics: fingerprint USA original render owner, modulo scheduling, and resource byte accessor
  Paths: `native/tools/audit_original_graphics.py`
- `f3ee9e9d42a71299e3640f3ea744e0d28d722f26` â€” BLOCKED: outside selected cluster; needs review. graphics docs: retail draw traversal, source render owner scheduling and native submission integration
  Paths: `native/docs/ORIGINAL_RETAIL_RENDER_PASSES.md`
- `6901f671d9e7122bc2102c1d63014d82c3aec65f` â€” BLOCKED: outside selected cluster; needs review. graphics handoff: original render scheduler, two-pass traversal, tests, runtime integration
  Paths: `native/parallel/handoffs/graphics-animation.md`
- `e9b7c15497fb73fee8737e983d1e49ad7a84c801` â€” BLOCKED: outside selected cluster; needs review. graphics integration test: original owner and both draw passes through native authentic-mesh scene API
  Paths: `native/tests/original_render_pipeline_test.cpp`
- `87d2d43c73cda8c57540b7c025688ddbd5061a52` â€” BLOCKED: outside selected cluster; needs review. graphics handoff: joined original render-owner, two-pass, native animated mesh submission test
  Paths: `native/parallel/handoffs/graphics-animation.md`
- `71582792e329f3c5f861505a88841164d43e9cce` â€” BLOCKED: outside selected cluster; needs review. graphics: fix source 0x26C9B8 branch polarity (zero means reject) across both passes
  Paths: `native/include/ae3/original_render_visibility.hpp`
- `fc5af6b7ab4b409f6ac4520fa4736b87996e123e` â€” BLOCKED: outside selected cluster; needs review. graphics: fix source 0x26C9B8 branch polarity (zero means reject) across both passes
  Paths: `native/src/original_render_visibility.cpp`
- `06be93908ae769868b8f2d3372da2cb468c0d67f` â€” BLOCKED: outside selected cluster; needs review. graphics: fix source 0x26C9B8 branch polarity (zero means reject) across both passes
  Paths: `native/include/ae3/original_render_pass.hpp`
- `493be2ad1c0e298f70f5018d90aa7d06b7278ff2` â€” BLOCKED: outside selected cluster; needs review. graphics: fix source 0x26C9B8 branch polarity (zero means reject) across both passes
  Paths: `native/src/original_render_pass.cpp`
- `24b5cbb31b71467f6d968f807769bdb5b3648867` â€” BLOCKED: outside selected cluster; needs review. graphics: fix source 0x26C9B8 branch polarity (zero means reject) across both passes
  Paths: `native/tests/original_render_visibility_test.cpp`
- `85e6a131e8647973c82388894653824b2d19fcd8` â€” BLOCKED: outside selected cluster; needs review. graphics: fix source 0x26C9B8 branch polarity (zero means reject) across both passes
  Paths: `native/tests/original_render_pass_test.cpp`
- `ab25ea20f1545cb48773f424a9974f0e822f2a93` â€” BLOCKED: outside selected cluster; needs review. graphics: fix source 0x26C9B8 branch polarity (zero means reject) across both passes
  Paths: `native/tests/original_render_pipeline_test.cpp`
- `e561800610a0afc95cbc00e1e7272b31cc05b37d` â€” BLOCKED: outside selected cluster; needs review. graphics: recover authentic retail GPU context slot setters/getters and stateful event application
  Paths: `native/include/ae3/original_render_context.hpp`
- `4288ed4b21f59da1ff819dc5678f4ae8c460a1a0` â€” BLOCKED: outside selected cluster; needs review. graphics: recover authentic retail GPU context slot setters/getters and stateful event application
  Paths: `native/src/original_render_context.cpp`
- `e6b8dd4f40aeb8ea54fc5f2f0df3f45955fcd6d3` â€” BLOCKED: outside selected cluster; needs review. graphics tests: original context register stores and fail-closed draw callback execution
  Paths: `native/tests/original_render_context_test.cpp`
- `1b5ef885298d35f7b210d8ad504631bcf1a831e7` â€” BLOCKED: outside selected cluster; needs review. graphics: implement original retail actor resource 0x26A854/0x26A510 lifecycle and flag accessors
  Paths: `native/include/ae3/original_render_resource.hpp`
- `5560efd0842bc1df378590ed70d755172e234b46` â€” BLOCKED: outside selected cluster; needs review. graphics: implement original retail actor resource 0x26A854/0x26A510 lifecycle and flag accessors
  Paths: `native/src/original_render_resource.cpp`
- `f5e66a48416d33e9dd12cd062048ecdd4e7a8648` â€” BLOCKED: outside selected cluster; needs review. graphics tests: verify original +0x110 resource allocation, preserved byte flags and nested selector
  Paths: `native/tests/original_render_resource_test.cpp`
- `86798cd3e604db47dc3d0da41a6491746809fd2b` â€” BLOCKED: outside selected cluster; needs review. graphics: reconstruct original 0x26831C model renderer entry timing and gated dispatch
  Paths: `native/include/ae3/original_render_actor_entry.hpp`
- `f6e19f967d2ee8df774e42263af2dfd7619711b7` â€” BLOCKED: outside selected cluster; needs review. graphics: reconstruct original 0x26831C model renderer entry timing and gated dispatch
  Paths: `native/src/original_render_actor_entry.cpp`
- `7d719a4ef0e6fba25c63c0b6c4c7f9567f0a1ad5` â€” BLOCKED: outside selected cluster; needs review. graphics tests: original 0x26831C render actor clock, callbacks and early display gates
  Paths: `native/tests/original_render_actor_entry_test.cpp`
- `f6b0265173b1c2fbfab322ccf25f6e3a39ac83ef` â€” BLOCKED: outside selected cluster; needs review. graphics: recover source-backed 0x2804F4 effect/resource preparation and test
  Paths: `native/include/ae3/original_render_effect.hpp`
- `127b414e4f77d07ef9cdb20a0f2b545d92a382a8` â€” BLOCKED: outside selected cluster; needs review. graphics: recover source-backed 0x2804F4 effect/resource preparation and test
  Paths: `native/src/original_render_effect.cpp`
- `d1d90a573e529cc734fa9762e373961b12e3adb9` â€” BLOCKED: outside selected cluster; needs review. graphics: recover source-backed 0x2804F4 effect/resource preparation and test
  Paths: `native/tests/original_render_effect_test.cpp`
- `56e34b19149333e7482b81b86137385bcf5970f6` â€” BLOCKED: outside selected cluster; needs review. graphics ELF audit: verify original context/resource/actor-entry/draw-wrapper instruction fingerprints
  Paths: `native/tools/audit_original_graphics.py`
- `acb4e1a61b7075ede338e7466260f2f8e96b29ee` â€” BLOCKED: outside selected cluster; needs review. graphics docs: correct cull polarity and prove retail context resource lifecycle timed draw entry effect chain
  Paths: `native/docs/ORIGINAL_RETAIL_RENDER_CONTEXT_AND_ACTOR_ENTRY.md`
- `1e0b2906cabad9167a63d5f6153ae66a9f83349e` â€” BLOCKED: outside selected cluster; needs review. graphics docs: correct original bounds branch and downstream actual state-write semantics
  Paths: `native/docs/ORIGINAL_RETAIL_RENDER_GATE_AND_INSTANCING.md`
- `2080685fc84e4b5cda92b02185b4eb70db7bf88e` â€” BLOCKED: outside selected cluster; needs review. graphics docs: correct original bounds branch and downstream actual state-write semantics
  Paths: `native/docs/ORIGINAL_RETAIL_RENDER_PASSES.md`
- `cb51f0cf41eae4488649dcf1860b1ac78c1598eb` â€” BLOCKED: outside selected cluster; needs review. graphics handoff: correct source cull polarity, implement retail context/resource actor clock effects, tests and integration
  Paths: `native/parallel/handoffs/graphics-animation.md`
- `985b80ee510ce42225cd30f2787cd168759ff31a` â€” BLOCKED: outside selected cluster; needs review. graphics: recover retail 0x280364/0x2803B4 model descriptor and packet binding pipeline
  Paths: `native/include/ae3/original_render_draw_packet.hpp`
- `a996221c286d169012ff724875109d09ddf89569` â€” BLOCKED: outside selected cluster; needs review. graphics: recover retail 0x280364/0x2803B4 model descriptor and packet binding pipeline
  Paths: `native/src/original_render_draw_packet.cpp`
- `9b3142172aff38c4a6f3e08546dfabbc3d4746ab` â€” BLOCKED: outside selected cluster; needs review. graphics: recover retail 0x280364/0x2803B4 model descriptor and packet binding pipeline
  Paths: `native/tests/original_render_draw_packet_test.cpp`
- `4d2b1118c786a4a192ddcf0882191efa91d28368` â€” BLOCKED: outside selected cluster; needs review. graphics: recover five-group original 0x3AF8F0 material packet assembly and native context integration
  Paths: `native/include/ae3/original_render_packet_assembly.hpp`
- `16182beed5f5d02a8367450475a3f3fa9d11d6eb` â€” BLOCKED: outside selected cluster; needs review. graphics: recover five-group original 0x3AF8F0 material packet assembly and native context integration
  Paths: `native/src/original_render_packet_assembly.cpp`
- `8bc1aa1ce0b30cf916b0c005e0b2c3127cc9eba5` â€” BLOCKED: outside selected cluster; needs review. graphics: recover five-group original 0x3AF8F0 material packet assembly and native context integration
  Paths: `native/tests/original_render_packet_assembly_test.cpp`
- `7247d65ba34b7cfdea0661d1d9d335d4bef8d1cb` â€” BLOCKED: outside selected cluster; needs review. graphics: retail executable SHA evidence for draw-to-packet and five-material-block assembler
  Paths: `native/tools/audit_original_graphics.py`
- `770c8781420bd195a33f835e261761c72b03684c` â€” BLOCKED: outside selected cluster; needs review. graphics: directly recover retail 0x3AF680 and 0x3AF600 fixed-stride packet resource copies
  Paths: `native/include/ae3/original_render_packet_copies.hpp`
- `e24dd7dd1df35a1e168efaf2fe7b6aeae82ab456` â€” BLOCKED: outside selected cluster; needs review. graphics: directly recover retail 0x3AF680 and 0x3AF600 fixed-stride packet resource copies
  Paths: `native/src/original_render_packet_copies.cpp`
- `790eb740a3efa060d2507b027ba4cd83e69abb20` â€” BLOCKED: outside selected cluster; needs review. graphics: directly recover retail 0x3AF680 and 0x3AF600 fixed-stride packet resource copies
  Paths: `native/tests/original_render_packet_copies_test.cpp`
- `25b67cf6f41a21339e306d0e1587e3e188171ea9` â€” BLOCKED: outside selected cluster; needs review. graphics: verify retail segment transfer 0x3AF600/0x3AF680 and memcpy source instructions
  Paths: `native/tools/audit_original_graphics.py`
- `a3d4babf571a887fe7018fa08748394d4570b4a0` â€” BLOCKED: outside selected cluster; needs review. graphics: document source verified genuine model render packet assembler and two original binary packet copies
  Paths: `native/docs/ORIGINAL_RETAIL_MODEL_DRAW_AND_PACKET_ASSEMBLY.md`
- `530a3e45cd27862c6c79bcd9a47e79688b465278` â€” BLOCKED: outside selected cluster; needs review. graphics handoff: recover genuine five-group render packet assembler, two original segment copies and tests
  Paths: `native/parallel/handoffs/graphics-animation.md`
- `1bc9ed20c83627e36e2969b0bf49b85914294981` â€” BLOCKED: outside selected cluster; needs review. graphics: recover original 0x3B3CE0/0x3B3D00 native 16-byte packet arena cursors
  Paths: `native/include/ae3/original_render_packet_arena.hpp`
- `d23a24abc3f8c54a8db3c9b32f52d1644a4404c8` â€” BLOCKED: outside selected cluster; needs review. graphics: recover original 0x3B3CE0/0x3B3D00 native 16-byte packet arena cursors
  Paths: `native/src/original_render_packet_arena.cpp`
- `e58bab4523977c6e9de8c4ae09a779cc9ea3b30c` â€” BLOCKED: outside selected cluster; needs review. graphics: recover original 0x3B3CE0/0x3B3D00 native 16-byte packet arena cursors
  Paths: `native/tests/original_render_packet_arena_test.cpp`
- `44ac9a1e280a2ee47dac9730c2137071d58818b5` â€” BLOCKED: outside selected cluster; needs review. graphics: pin source SHA ranges for genuine bidirectional packet arena allocator
  Paths: `native/tools/audit_original_graphics.py`
- `8c131b3ed6a10f2e991ed4ebeed163228b43b5e8` â€” BLOCKED: outside selected cluster; needs review. graphics docs: retail 16-byte two-ended packet allocation cursor recovery and tests
  Paths: `native/docs/ORIGINAL_RETAIL_MODEL_DRAW_AND_PACKET_ASSEMBLY.md`
- `3460ea448eac6a97a3de753c770b4466b5dc9b7d` â€” BLOCKED: outside selected cluster; needs review. graphics handoff: original two-ended 16-byte graphics packet arena cursor port and verified native tests
  Paths: `native/parallel/handoffs/graphics-animation.md`
- `862ebd31d335e3954bc86ae3c6899948ab9dd0d0` â€” BLOCKED: outside selected cluster; needs review. graphics: recover three original transformed packet-group layouts and retail group-2 scalar math
  Paths: `native/include/ae3/original_render_transformed_segments.hpp`
- `610519db7061df28425db8d61bc1c100cc2ccead` â€” BLOCKED: outside selected cluster; needs review. graphics: recover three original transformed packet-group layouts and retail group-2 scalar math
  Paths: `native/src/original_render_transformed_segments.cpp`
- `8cf8f4f985aeb547f2b473301da95d36d92703bd` â€” BLOCKED: outside selected cluster; needs review. graphics test: original 0/2/3 record layouts, group2 scalar clamps and genuine callback guards
  Paths: `native/tests/original_render_transformed_segments_test.cpp`
- `4295a2f59567134689ba3a6cb91ad9464e0b59e8` â€” BLOCKED: outside selected cluster; needs review. graphics: original retail packet header typed native memory interface
  Paths: `native/include/ae3/original_render_packet_storage.hpp`
- `7a779047b32f4ef78a77e5a0016c434d639182e9` â€” BLOCKED: outside selected cluster; needs review. graphics: implement original reset setup first quadword and rear packet header storage
  Paths: `native/src/original_render_packet_storage.cpp`
- `96bea9ec8190f089e5950a56fbabad94322a5ace` â€” BLOCKED: outside selected cluster; needs review. graphics tests: genuine quadword packet initialization and allocated header identity
  Paths: `native/tests/original_render_packet_storage_test.cpp`
- `eb2354e7c3b858163285432babf42ce042faa86a` â€” BLOCKED: outside selected cluster; needs review. graphics: source recover 0x27C614 five model source streams and 0x27C520 supplemental range
  Paths: `native/include/ae3/original_render_model_streams.hpp`
- `e8a344cfe70bbce923c0a75134616ee2a88893c1` â€” BLOCKED: outside selected cluster; needs review. graphics: source recover 0x27C614 five model source streams and 0x27C520 supplemental range
  Paths: `native/src/original_render_model_streams.cpp`
- `1d47af9b22b39733e825bdd2fe78136966985953` â€” BLOCKED: outside selected cluster; needs review. graphics tests: five original converter input stream counts, record strides and invalid pointer pairs
  Paths: `native/tests/original_render_model_streams_test.cpp`
- `19eb749b139759e98c1ead338aebb54f4c7dcd26` â€” BLOCKED: outside selected cluster; needs review. graphics: source SHA and IEEE754 verification for original model streams, three packet groups, original packet initialization
  Paths: `native/tools/audit_original_graphics.py`
- `29eed5ddf4af8a5fe7c6fdc0d2d7cfc6a21fa6c8` â€” BLOCKED: outside selected cluster; needs review. graphics docs: verified retail transformed record and five model-source streams recovery
  Paths: `native/docs/ORIGINAL_RETAIL_TRANSFORMED_PACKET_RECOVERY.md`
- `9cc1ac71581c94ab1f1835e212d37caa07e17e5c` â€” BLOCKED: outside selected cluster; needs review. graphics handoff: retail model stream strides, packet scalar/quadword source, all 17 sanitizer suites
  Paths: `native/parallel/handoffs/graphics-animation.md`
- `c491b6bea5c33da7d7650737e9ee25982c145e64` â€” BLOCKED: outside selected cluster; needs review. graphics: restore original packet +0x38 graphics owner store in 0x3AF8F0 call order
  Paths: `native/include/ae3/original_render_packet_assembly.hpp`
- `8b74e631a9ca9ec64535747705daec20da88f376` â€” BLOCKED: outside selected cluster; needs review. graphics: restore original packet +0x38 graphics owner store in 0x3AF8F0 call order
  Paths: `native/src/original_render_packet_assembly.cpp`
- `732caa38d684085742ed6a3af903be9eccf2dd3b` â€” BLOCKED: outside selected cluster; needs review. graphics regression: verify original graphics owner assignment between packet reset and setup
  Paths: `native/tests/original_render_packet_assembly_test.cpp`
- `733102e36fab60d8653bac668770b81213480a65` â€” BLOCKED: outside selected cluster; needs review. graphics: expose original rear packet reservation for native source call ordering
  Paths: `native/include/ae3/original_render_packet_storage.hpp`
- `28d2a33a082d2d8397eb7a2bf2c54b8c5fb91d80` â€” BLOCKED: outside selected cluster; needs review. graphics: preserve original packet reservation and correct reset owner-bind setup sequence
  Paths: `native/src/original_render_packet_storage.cpp`
- `e812bdf897bf9a1ff1417909b884e0f9de6544fe` â€” BLOCKED: outside selected cluster; needs review. graphics joined test: original model source through packet owner/reset/descriptor group transform and native context
  Paths: `native/tests/original_render_model_packet_bridge_test.cpp`
- `9d2e489bc9fa8abafbbc4068b1dced92dac26478` â€” BLOCKED: outside selected cluster; needs review. graphics docs: original 0x3AF920 missing packet owner store corrected, 18 regression suites
  Paths: `native/docs/ORIGINAL_RETAIL_TRANSFORMED_PACKET_RECOVERY.md`
- `62bdd9b10a648638f3d036bcc8c698ec8a6370c1` â€” BLOCKED: outside selected cluster; needs review. graphics handoff: fix original packet owner store callback, verify complete 18-suite GCC and sanitizer runs
  Paths: `native/parallel/handoffs/graphics-animation.md`

Tests absent from baseline CMake (runtime conditional manifest also needs review): `native/tests/original_render_actor_entry_test.cpp`, `native/tests/original_render_context_test.cpp`, `native/tests/original_render_draw_packet_test.cpp`, `native/tests/original_render_effect_test.cpp`, `native/tests/original_render_instances_test.cpp`, `native/tests/original_render_model_packet_bridge_test.cpp`, `native/tests/original_render_model_streams_test.cpp`, `native/tests/original_render_owner_schedule_test.cpp`, `native/tests/original_render_packet_arena_test.cpp`, `native/tests/original_render_packet_assembly_test.cpp`, `native/tests/original_render_packet_copies_test.cpp`, `native/tests/original_render_packet_storage_test.cpp`, `native/tests/original_render_pass_test.cpp`, `native/tests/original_render_pipeline_test.cpp`, `native/tests/original_render_resource_test.cpp`, `native/tests/original_render_selector_test.cpp`, `native/tests/original_render_transformed_segments_test.cpp`, `native/tests/original_render_visibility_test.cpp`

### assets

Tip `a63a70f063658fb3282ca099b6d33f7b55e880bb`; merge base `d483436416c14927851853f01c6ad66fe4cfbbf2`; 70 commits; 37 changed paths.

Decision: ACCEPTED for cluster validation.

- `c029f9823469cef64f55fd079e58848192baacd4` â€” ACCEPTED for cluster validation. assets: add owner-stable retail PCK resource table and typed lookups (0x26D24C)
  Paths: `native/include/ae3/original_resource_package.hpp`
- `776162f2daad5bf1d8cd47d082b38aefd554bbd8` â€” ACCEPTED for cluster validation. assets: connect original ISO/VFI to owned typed PCK resource packages
  Paths: `native/include/ae3/original_resource_vfi_mount.hpp`
- `2c3ce3f122657448df616dd130d1d9673ffa1224` â€” ACCEPTED for cluster validation. assets: test authenticated EXDB package layout, stable views and malicious offsets
  Paths: `native/tests/original_resource_package_test.cpp`
- `8b6aaaa9c6a2382f566e414005e941e14189048d` â€” ACCEPTED for cluster validation. assets: handoff source offsets, ownership APIs, verified tests and stage-loading blockers
  Paths: `native/parallel/handoffs/assets.md`
- `827048c063cb837a8d202c898a8d2c3d077d592e` â€” ACCEPTED for cluster validation. assets: correct literal Markdown formatting in integration handoff
  Paths: `native/parallel/handoffs/assets.md`
- `05edbc68f859900865013f1b6078f6d50c0896cc` â€” ACCEPTED for cluster validation. assets: define original VFI SZ raw-DEFLATE resource decoding contract
  Paths: `native/include/ae3/original_resource_sz.hpp`
- `22577197233c7dacb6f1429d3e84fae7d734d80e` â€” ACCEPTED for cluster validation. assets: implement bounded native original SZ expansion with strict raw-DEFLATE validation
  Paths: `native/src/original_resource_sz.cpp`
- `2cd07dc2fc1fa4aa2482f370d4daea84189547dc` â€” ACCEPTED for cluster validation. assets: verify raw SZ decompression and full compressed PCK resource chain
  Paths: `native/tests/original_resource_sz_test.cpp`
- `fc1e061ef20d9de61aed0323d6a17c4a0d512c5a` â€” ACCEPTED for cluster validation. assets: expand authentic SZ VFI members before typed PCK parsing; retain stored-byte access
  Paths: `native/include/ae3/original_resource_vfi_mount.hpp`
- `0edd240d9a87f25f899671742662d52d442c28ed` â€” ACCEPTED for cluster validation. assets: record source-backed native SZ loading, sanitizer tests and CMake integration request
  Paths: `native/parallel/handoffs/assets.md`
- `002c4391ea7928b83ba18097f9d5b2dd5e31967b` â€” ACCEPTED for cluster validation. assets: add private retail EXDB PCK regression against documented original offsets
  Paths: `native/tests/original_resource_package_retail_test.cpp`
- `4df916bb5deefd55fa74b696e4da45ab2d61b873` â€” ACCEPTED for cluster validation. assets: record optional private retail EXDB regression and SHA validation dependency
  Paths: `native/parallel/handoffs/assets.md`
- `c8bff2b4fa1344c2d6c6809410686a00daa8f805` â€” ACCEPTED for cluster validation. assets: support original navigation SZ optional trailers with explicit strict mode
  Paths: `native/include/ae3/original_resource_sz.hpp`
- `88281e6922b832c622f403b7885a037512e8e712` â€” ACCEPTED for cluster validation. assets: support original navigation SZ optional trailers with explicit strict mode
  Paths: `native/src/original_resource_sz.cpp`
- `1fb67144408b0b4826af97a36e2a40c85edcde35` â€” ACCEPTED for cluster validation. assets: support original navigation SZ optional trailers with explicit strict mode
  Paths: `native/include/ae3/original_resource_vfi_mount.hpp`
- `be910d7d1013b1053552779791c21602e38b34b1` â€” ACCEPTED for cluster validation. assets: support original navigation SZ optional trailers with explicit strict mode
  Paths: `native/tests/original_resource_sz_test.cpp`
- `2a338aec7a1e115a922f4afb6f83c042ba755f9b` â€” ACCEPTED for cluster validation. assets: inventory original USA stage VFI members with exact source offsets and safe lookup
  Paths: `native/include/ae3/original_resource_stage_catalog.hpp`
- `ce5570a8904b6c7953f75c4292730d607cbc2fc6` â€” ACCEPTED for cluster validation. assets: test original-stage inventory discovery, exact resource paths, duplicate and traversal rejection
  Paths: `native/tests/original_resource_stage_catalog_test.cpp`
- `da91925eb3743e675839d8d0459642eb7dd99ee8` â€” ACCEPTED for cluster validation. assets: add native original ISO stage inventory probe with opt-in PCK member validation
  Paths: `native/src/original_resource_stage_probe.cpp`
- `ecd085b02ecbd0411ac224e1dbf1c82578177403` â€” ACCEPTED for cluster validation. assets: regression for original NavGraph SZ PCK 0x4F0A5C typed resource chain
  Paths: `native/tests/original_resource_sz_test.cpp`
- `3f31be2dbbb15754086e50d315b7aa20690501f6` â€” ACCEPTED for cluster validation. assets: document recovered original nav source compatibility, stage inventory probe and local tests
  Paths: `native/parallel/handoffs/assets.md`
- `e73d1b5c7b0f6c2deffb6cd0ba137c3f9859a9a9` â€” ACCEPTED for cluster validation. assets: retain original VFI stage directory identifiers instead of launcher-only naming assumptions
  Paths: `native/include/ae3/original_resource_stage_catalog.hpp`
- `ff708ff3fb7cadcaeedbd467f8deed6c79cc8d5a` â€” ACCEPTED for cluster validation. assets: regression for authentic arbitrary VFI stage directory names
  Paths: `native/tests/original_resource_stage_catalog_test.cpp`
- `51abacaf6a984c2b5095e5e217699efda0cc9dff` â€” ACCEPTED for cluster validation. assets: note source VFI stage-name correction and updated focused regression
  Paths: `native/parallel/handoffs/assets.md`
- `d22511c2f49e806f747907b64b62a8c2f3749330` â€” ACCEPTED for cluster validation. assets: finalize corrected stage-catalog sanitizer test status
  Paths: `native/parallel/handoffs/assets.md`
- `5ca2ab94bf2ba5db5a218ee1e86827f502d43c8b` â€” ACCEPTED for cluster validation. assets: own exact per-stage VFI/PCK/EXDB resources across consumer lifetimes
  Paths: `native/include/ae3/original_resource_stage_scope.hpp`
- `9c0ba0a9558299b4057ac4124302d2977dcf4a69` â€” ACCEPTED for cluster validation. assets: test original-format ISO/VFI/SZ/PCK stage allocation lifetimes and EXDB rollback
  Paths: `native/tests/original_resource_stage_scope_test.cpp`
- `c5e56e6f57a05924fa4c4249b6a558e078e62e24` â€” ACCEPTED for cluster validation. assets: verify owner-private USA ISO -> woods_c EXDB data, original attributes and stable stage lifetime
  Paths: `native/tests/original_resource_stage_scope_retail_test.cpp`
- `dd129b7418e6a08c3594d93ab6111a4592ddb618` â€” ACCEPTED for cluster validation. assets: verify authentic VFI parent-linked nested directories through stage PCK ownership
  Paths: `native/tests/original_resource_vfi_folder_owner_test.cpp`
- `16871aee24ca3b12136313c6b7cef34d0a434b21` â€” ACCEPTED for cluster validation. assets: handoff real stage ownership, nested VFI folder source tests, optional private EXDB round-trip
  Paths: `native/parallel/handoffs/assets.md`
- `487336411e85132bac0d1c71a83055226c91f099` â€” ACCEPTED for cluster validation. assets: enforce original 0x1E nav action slots and full 0x20 indexed source entry bounds
  Paths: `native/src/original_stage_spatial_loader.cpp`
- `05be3e4cc03b321e6c26ce05b627f83cff3552e0` â€” ACCEPTED for cluster validation. assets: regression for three-slot retail NavGraph actions and complete source extra-record bounds
  Paths: `native/tests/original_stage_spatial_loader_test.cpp`
- `c4d36c2a3827047a8665c2c13fde3e5e253cd3bb` â€” ACCEPTED for cluster validation. assets: handoff source-verified NavGraph descriptor bounds, new regressions and unverified integration
  Paths: `native/parallel/handoffs/assets.md`
- `4d5d2506127c0fdd5c2d9599b333dc74cef57f86` â€” ACCEPTED for cluster validation. assets: preserve source PCK resource type strings without breaking raw offset API
  Paths: `native/include/ae3/pck_archive.hpp`
- `48c5abe2aeb6a4e836f6b7b63ac4ce120555b2ce` â€” ACCEPTED for cluster validation. assets: fix generic PCK absolute name offsets and retain original member type text from retail files
  Paths: `native/src/pck_archive.cpp`
- `f9b8c57e74af7bf4fe7ddc29add15e7919d2e596` â€” ACCEPTED for cluster validation. assets: align synthetic PCK test with five verified retail absolute-name/type layouts
  Paths: `native/tests/pck_archive_test.cpp`
- `d5f0d5cc0827ba3c24379edd03ef2bba1c67e704` â€” ACCEPTED for cluster validation. assets: differential-test five SHA-verified original stage PCK tables and every resource byte
  Paths: `native/tests/original_pck_stage_retail_test.cpp`
- `bf0fa303a91bd1523b07df5c62972ee5a7e060cb` â€” ACCEPTED for cluster validation. assets: SHA-pin original stage-init lookup chain and five retail PCK/nav source packages
  Paths: `native/tools/verify_original_stage_resource_initialization.py`
- `e94570bfc946ce171b810a3cc37499ab1f00501e` â€” ACCEPTED for cluster validation. assets: expose authentic ELF stage-init lookup order and exact VFI member candidates
  Paths: `native/include/ae3/original_resource_stage_sequence.hpp`
- `30828575e467092ca61d55075d1c35456cc8b473` â€” ACCEPTED for cluster validation. assets: verify recovered 0x22D7B4 lookup order against exact VFI candidates and ambiguities
  Paths: `native/tests/original_resource_stage_sequence_test.cpp`
- `54a7baf19f337fb6fadc3d50dea5081478bb7b82` â€” ACCEPTED for cluster validation. assets: correct span indexing and guard source candidate index after focused compiler test
  Paths: `native/include/ae3/original_resource_stage_sequence.hpp`
- `8d36aabcfbe97f54f31c44ecbeaa73138a05cf54` â€” ACCEPTED for cluster validation. assets: recover original stage source name lookup loop at 0x24D4F0 into native view
  Paths: `native/include/ae3/original_resource_named_lookup.hpp`
- `ff941b1a51d0175540229ccaa396d06ab89d8207` â€” ACCEPTED for cluster validation. assets: test original 0x24D4F0 name lookup exactness, first-match order, and borrowed payload
  Paths: `native/tests/original_resource_named_lookup_test.cpp`
- `b67126053291e26d22b7d7e35669bdce23ce95ec` â€” ACCEPTED for cluster validation. assets: record original retail stage-init source VAs, hashes and woods_c nav/PCK evidence
  Paths: `native/reference/original_stage_initialization_resource_chain.json`
- `f14b07a8f5e4efc6ffe5fd4a436e6c573bf52d49` â€” ACCEPTED for cluster validation. assets: hand off fresh owner-private ELF/PCK/nav proof, recovered original stage startup and tested readers
  Paths: `native/parallel/handoffs/assets.md`
- `5a46c3778e80885c451e3ae09b3953cac8e7951e` â€” ACCEPTED for cluster validation. assets: preserve original pre-lookup wildcard source registration call 0x22D7F8
  Paths: `native/include/ae3/original_resource_stage_sequence.hpp`
- `ff158043cebcb91473878d83083d2e4c3c43b773` â€” ACCEPTED for cluster validation. assets: implement source-verified wildcard/strict selection branch of 0x25182C resource registry
  Paths: `native/include/ae3/original_resource_registration_filter.hpp`
- `ea30282aae09148892b648556b6de1abb75c7d32` â€” ACCEPTED for cluster validation. assets: test original 0x25182C wildcard and name filtering without invented archive semantics
  Paths: `native/tests/original_resource_registration_filter_test.cpp`
- `427504e215939ef959f67bce88666520167e4a4a` â€” ACCEPTED for cluster validation. assets: verify ELF wildcard registration upstream of ordered stage name lookups
  Paths: `native/tools/verify_original_stage_resource_initialization.py`
- `7dbd834f10def96a8aafedd59e4a7e9f68d63da8` â€” ACCEPTED for cluster validation. assets: correct literal NUL token in original wildcard ELF source verifier
  Paths: `native/tools/verify_original_stage_resource_initialization.py`
- `0b6c1ee0afff357f189bfe9efef632355b23c177` â€” ACCEPTED for cluster validation. assets: extend original stage source evidence with upstream wildcard registration and 442 audited opcodes
  Paths: `native/reference/original_stage_initialization_resource_chain.json`
- `6adc8baf718e60b462fe94e27f0b2b14fc36e941` â€” ACCEPTED for cluster validation. assets: hand off verified upstream wildcard registration and original stage boot ordering
  Paths: `native/parallel/handoffs/assets.md`
- `2fb09d825187fb10c591ab60d25972a06fce179e` â€” ACCEPTED for cluster validation. assets: correct 0x2519FC selector target to record +4 type suffix, not resource name
  Paths: `native/include/ae3/original_resource_registration_filter.hpp`
- `9662aba4570ec63ea9bc890fcd7a1d99673758d2` â€” ACCEPTED for cluster validation. assets: reconstruct 0x24D858 filename split, typed record creation and 0x25182C filtered append
  Paths: `native/include/ae3/original_resource_record_registration.hpp`
- `219aac5f783cef91013f50f6a034c2cbcc1e70ed` â€” ACCEPTED for cluster validation. assets: test source 0x24D858 first-dot name/type split, 0x25182C filter and first-match lookup
  Paths: `native/tests/original_resource_record_registration_test.cpp`
- `9fd287e331cfe03b52a5f4d2c292de7ed76aa8ab` â€” ACCEPTED for cluster validation. assets: wire original source VFI stage ownership into 0x24D858/0x25182C/0x24D4F0 chain
  Paths: `native/include/ae3/original_resource_stage_registration.hpp`
- `16eddb1f2ba9ac756698483964dbba3418aff993` â€” ACCEPTED for cluster validation. assets: connect synthetic ISO/VFI/SZ/PCK stage owner to original 0x25182C registry and lookup tests
  Paths: `native/tests/original_resource_stage_scope_test.cpp`
- `123b2dfd86fb58e9c8b7afb14a93dc6b6aa6a8ad` â€” ACCEPTED for cluster validation. assets: add private genuine woods_c record creation/filter/lookup test for all four stage-init keys
  Paths: `native/tests/original_resource_record_registration_retail_test.cpp`
- `13cf72c8f604a66ed160e4b018c9bf96053f3ad6` â€” ACCEPTED for cluster validation. assets: verify original 0x24D858/0x25182C/0x24D4F0 ELF instructions and nine SHA-pinned woods_c files
  Paths: `native/tools/verify_original_resource_record_registration.py`
- `a55902e849197bde77351d2c063441a9c30fc067` â€” ACCEPTED for cluster validation. assets: correct 0x2519FC test semantics to source extension/type string comparisons
  Paths: `native/tests/original_resource_registration_filter_test.cpp`
- `d02622e6e4b65ed4b8cfe54a1055a79fa81e1133` â€” ACCEPTED for cluster validation. assets: evidence for 0x24D858 source record construction, 543 unique verified words, extension filtering
  Paths: `native/reference/original_stage_initialization_resource_chain.json`
- `8527f44c20dc13cc73162645f8b21a1830ee6844` â€” ACCEPTED for cluster validation. assets: handoff recovered first-dot original stage resource registrar, private retail tests and remaining directory traversal
  Paths: `native/parallel/handoffs/assets.md`
- `683e4787851f8a02c68ab2a43218e102d02a42b9` â€” ACCEPTED for cluster validation. assets: recover bytewise source directory key ordering from 0x627E60/0x49C150 over genuine VFI children
  Paths: `native/include/ae3/original_resource_directory_order.hpp`
- `c914d16e72d634d62c84dee0fd06d04c2658f931` â€” ACCEPTED for cluster validation. assets: test source 0x627E60 directory ordering and actual direct-child VFI identities
  Paths: `native/tests/original_resource_directory_order_test.cpp`
- `bf0797beb4c8792ab95181a881f8ff1d19d21581` â€” ACCEPTED for cluster validation. assets: pin original directory RB-tree traversal and bytewise key comparator for resource registration
  Paths: `native/tools/verify_original_resource_record_registration.py`
- `b9c07514650a8d4e7a956ab8146302fd022af7bc` â€” ACCEPTED for cluster validation. assets: correct original ordered-directory lookup JAL site from decoded ELF
  Paths: `native/tools/verify_original_resource_record_registration.py`
- `2517a73929b97e057afbb29d759105bb61683ac9` â€” ACCEPTED for cluster validation. assets: compose recovered directory key traversal, true VFI bytes, resource records and original name lookup
  Paths: `native/include/ae3/original_resource_stage_directory_registration.hpp`
- `d0dfa9c27f8829f01f2afe312088ee78693f08e5` â€” ACCEPTED for cluster validation. assets: test ordered DirInfo-like stage inventory through VFI/SZ/PCK and original 0x24D4F0 lookup
  Paths: `native/tests/original_resource_stage_scope_test.cpp`
- `bcd221a316e759fa04b003af5d7dc60ed2437a13` â€” ACCEPTED for cluster validation. assets: pin directory-tree source traverser, comparator and 646 unique original stage-init words
  Paths: `native/reference/original_stage_initialization_resource_chain.json`
- `a63a70f063658fb3282ca099b6d33f7b55e880bb` â€” ACCEPTED for cluster validation. assets: hand off original DirInfo bytewise map traversal and full VFI-to-stage-registry connected tests
  Paths: `native/parallel/handoffs/assets.md`

Tests absent from baseline CMake (runtime conditional manifest also needs review): `native/tests/original_pck_stage_retail_test.cpp`, `native/tests/original_resource_directory_order_test.cpp`, `native/tests/original_resource_named_lookup_test.cpp`, `native/tests/original_resource_package_retail_test.cpp`, `native/tests/original_resource_package_test.cpp`, `native/tests/original_resource_record_registration_retail_test.cpp`, `native/tests/original_resource_record_registration_test.cpp`, `native/tests/original_resource_registration_filter_test.cpp`, `native/tests/original_resource_stage_catalog_test.cpp`, `native/tests/original_resource_stage_scope_retail_test.cpp`, `native/tests/original_resource_stage_scope_test.cpp`, `native/tests/original_resource_stage_sequence_test.cpp`, `native/tests/original_resource_sz_test.cpp`, `native/tests/original_resource_vfi_folder_owner_test.cpp`

### lua

Tip `13f9c38ee0209fc604cfaf228a32c38481f0f548`; merge base `d483436416c14927851853f01c6ad66fe4cfbbf2`; 53 commits; 11 changed paths.

Decision: ACCEPTED for cluster validation.

- `3182d1de4db1d571978b0583035b10d5d28fb315` â€” ACCEPTED for cluster validation. lua: define verified Lua-to-native binding and callback dispatch interfaces
  Paths: `native/include/ae3/lua_stage.hpp`
- `2bc08990f10d3cc5752cd8c3de7f4b04b140bad8` â€” ACCEPTED for cluster validation. lua: dispatch verified original Lua callbacks to native runtime; reject unbacked retail flags and stage state
  Paths: `native/src/lua_stage.cpp`
- `9f1b295525725c45354c2b802a9d137c2e59c2db` â€” ACCEPTED for cluster validation. lua: keep unrecovered stage variable argument contract opaque
  Paths: `native/include/ae3/lua_stage.hpp`
- `ffd502126015a23ec5cf14fe49565202836951ca` â€” ACCEPTED for cluster validation. lua: dispatch unrecovered stage_var_get_int signature without guessing argument type
  Paths: `native/src/lua_stage.cpp`
- `1ef49bbffb77f6cad7d7772886c4b533cff6f80a` â€” ACCEPTED for cluster validation. lua tests: exercise setup-to-manager call chain, verified native function dispatch and Lua event callbacks
  Paths: `native/tests/lua_runtime_test.cpp`
- `cf63d235807eb390f75f8ceb7fdfd3ef786ce80f` â€” ACCEPTED for cluster validation. lua: audit original 5.0 GETGLOBAL/SETGLOBAL dependencies without extracting copyrighted scripts
  Paths: `native/tools/audit_original_lua50_calls.py`
- `5c738aa12f14564a913dc89c91c525d4cb5fafe8` â€” ACCEPTED for cluster validation. lua tests: verify retail opcode global audit, nested prototypes and corruption rejection
  Paths: `native/tools/test_audit_original_lua50_calls.py`
- `c6388336dd74ddaebe508a04f6752ea77daf9c61` â€” ACCEPTED for cluster validation. lua handoff: evidence, native API, tests, integration wiring and original-script blockers
  Paths: `native/parallel/handoffs/lua.md`
- `b80c08cfe356e87afebc1f30da9bfe54659c5b4d` â€” ACCEPTED for cluster validation. lua: reject unrepresentable native callback argument types and restore Lua stack on argument error
  Paths: `native/src/lua_stage.cpp`
- `1dc87269ea6c11545a4c20036a1562c1e1ea8a82` â€” ACCEPTED for cluster validation. lua tests: retail-bytecode mode remains strict across multi-chunk script loading
  Paths: `native/tests/retail_lua50_test.cpp`
- `f6ea149fe7301e968b6c2d12edbbdf64d919fcf2` â€” ACCEPTED for cluster validation. lua handoff: add strict multi-chunk test and exact audit test provenance
  Paths: `native/parallel/handoffs/lua.md`
- `675030ea9ac1187c09b94dd7a9d464277bf92da5` â€” ACCEPTED for cluster validation. lua: track recursive table identity when marshaling original stage parameters
  Paths: `native/include/ae3/lua_stage.hpp`
- `e82a0e65e0f5d69d0c8718256581dddb0e987f43` â€” ACCEPTED for cluster validation. lua: share a bounded conversion budget across nested original Lua tables
  Paths: `native/include/ae3/lua_stage.hpp`
- `479c1bbd3b8402e6df42e7168c9d699488970944` â€” ACCEPTED for cluster validation. lua: preserve original Lua binding arguments or fail explicitly; guard cycles, invalid keys and oversized values
  Paths: `native/src/lua_stage.cpp`
- `20002339380cdfeb4948f2c1cf9bc561222a787b` â€” ACCEPTED for cluster validation. lua: reject mixed numeric and string table keys rather than changing their Lua types
  Paths: `native/src/lua_stage.cpp`
- `08bb1e56d3394012a46ea3d0c771d536385cb321` â€” ACCEPTED for cluster validation. lua: expose lossless Lua 5.0 C-stack callback registration for recovered engine functions
  Paths: `native/include/ae3/lua_stage.hpp`
- `c23b87e359f3b3d3fd5bca9acdb813fef6d74848` â€” ACCEPTED for cluster validation. lua: dispatch original native Lua functions directly on unmodified Lua 5.0 stack
  Paths: `native/src/lua_stage.cpp`
- `2e80b68a0032b97cbe484560af171b72e1a05f39` â€” ACCEPTED for cluster validation. lua tests: verify raw Lua table/closure identity and strict rejection of lossy typed bindings
  Paths: `native/tests/lua_runtime_test.cpp`
- `c5067982433f3962cef1f424939a9b3e7e0e5cb2` â€” ACCEPTED for cluster validation. lua: correct original Lua 5.0 iABx index decoding to bit 6 and fix bytecode fixtures
  Paths: `native/tools/audit_original_lua50_calls.py`
- `9776e9f25557b81ba6e62ab140aca47c9f5c6de7` â€” ACCEPTED for cluster validation. lua: correct original Lua 5.0 iABx index decoding to bit 6 and fix bytecode fixtures
  Paths: `native/tools/test_audit_original_lua50_calls.py`
- `da3d215fab772dee97b452bd9af1b3bf6f48b404` â€” ACCEPTED for cluster validation. lua: recover conservative direct-global call sites from original 5.0 bytecode and track per-function evidence
  Paths: `native/tools/audit_original_lua50_calls.py`
- `737c21ef37d8f5d848c78840ccfb6e25d2098945` â€” ACCEPTED for cluster validation. lua tests: original 5.0 bit positions, direct calls, MOVE aliases and branch-safe call inventory
  Paths: `native/tools/test_audit_original_lua50_calls.py`
- `66d9849a1fd7a99cc76fc04b4a854e63646e8e27` â€” ACCEPTED for cluster validation. lua: enter retail strict mode only after original Lua 5.0 bytecode validates
  Paths: `native/src/lua_stage.cpp`
- `fd8fdb29a1f01d9c2716e2119079c039df9d8daf` â€” ACCEPTED for cluster validation. lua tests: invalid retail bytecode must not poison subsequent Lua 5.0 source execution
  Paths: `native/tests/retail_lua50_test.cpp`
- `802860f4fde0f3822f217e711a11f1bb078d6143` â€” ACCEPTED for cluster validation. lua: invalidate call register conservatively after each Lua 5.0 CALL
  Paths: `native/tools/audit_original_lua50_calls.py`
- `7db7179d744c6de90c4036d84f4b9aa2b1ab2531` â€” ACCEPTED for cluster validation. lua handoff: original 5.0 opcode fix, raw binding dispatch, six tests and remaining authentic engine blockers
  Paths: `native/parallel/handoffs/lua.md`
- `fca7444da054db969286736b03c2c2cd6ed401dc` â€” ACCEPTED for cluster validation. lua: source-register-aware MIPS CreateBind scan with VA validation, delay slots and conservative flow barriers
  Paths: `native/tools/map_original_lua_registrations.py`
- `63286fee330812543100220af9b3a54c623b2118` â€” ACCEPTED for cluster validation. lua tests: eight MIPS original registration provenance tests without retail game bytes
  Paths: `native/tools/test_map_original_lua_registrations.py`
- `a787288236cb49145a06ad20d34253389806f4b2` â€” ACCEPTED for cluster validation. lua: reconcile verified ELF Lua CreateBind registrations with original 5.0 stage call dependencies
  Paths: `native/tools/reconcile_original_lua_bindings.py`
- `512a3c7b69850c6e9851d4f25329e40097643cff` â€” ACCEPTED for cluster validation. lua tests: cross-reference original call sites and detect missing native binding candidates
  Paths: `native/tools/test_reconcile_original_lua_bindings.py`
- `f3391447da297de32e34c3af3ec964fe1ae1c3f9` â€” ACCEPTED for cluster validation. lua: inspect 11 published original flag binding vtables as raw source pointers; avoid guessed callback semantics
  Paths: `native/tools/map_original_lua_registrations.py`
- `0cd00bb4e8204f613e4cbb43257e57ec1faadd29` â€” ACCEPTED for cluster validation. lua tests: validate authentic-source vtable pointer inventory and unavailable sections
  Paths: `native/tools/test_map_original_lua_registrations.py`
- `fefc3d4ff62239d1252e6bd075a0b801f6035911` â€” ACCEPTED for cluster validation. lua handoff: 19 offline tests, static registration/vtable evidence, reconciler and genuine-script blockers
  Paths: `native/parallel/handoffs/lua.md`
- `59a7e150f788c8f2a1a2dbb7911fa0da5d6f8672` â€” ACCEPTED for cluster validation. lua: accept original Lua 5.0 OP_CLOSURE (35) during stage dependency scanning
  Paths: `native/tools/audit_original_lua50_calls.py`
- `93856be12716abc239e32777e4c4a1961fa2a57c` â€” ACCEPTED for cluster validation. lua tests: original Lua 5.0 stage CLOSURE opcode regression and invalid opcode rejection
  Paths: `native/tools/test_audit_original_lua50_calls.py`
- `8b4cab6d0b21ae6dea8970f26abecba605c481d9` â€” ACCEPTED for cluster validation. lua: conservatively kill MIPS constant provenance on all clobbering instructions
  Paths: `native/tools/map_original_lua_registrations.py`
- `2d449ef25dc7f98b14320f040c18720894c900e9` â€” ACCEPTED for cluster validation. lua tests: reject MIPS false registration matches after immediate, R-type and coprocessor clobbers
  Paths: `native/tools/test_map_original_lua_registrations.py`
- `42a9a1f86bea7f3d78c20c636ccbdd409d54e196` â€” ACCEPTED for cluster validation. lua: fix native callback failure diagnostics to use Lua 5.0-supported format specifiers
  Paths: `native/src/lua_stage.cpp`
- `1fd4800ed40a0b2cf00e1812436ac396cd18b81d` â€” ACCEPTED for cluster validation. lua tests: exercise actual Lua 5.0 error formatter when recovered native callback fails
  Paths: `native/tests/lua_runtime_test.cpp`
- `dbdc141f5f6a714c878a2d7895827590b6d57020` â€” ACCEPTED for cluster validation. lua: correlate published retail Lua vtable leads with real script references without inventing callback VAs
  Paths: `native/tools/reconcile_original_lua_bindings.py`
- `c6bd6256a593a5931fb249becc31bc94321f9aaa` â€” ACCEPTED for cluster validation. lua tests: vtable-only candidates remain separate from verified CreateBind and callback implementations
  Paths: `native/tools/test_reconcile_original_lua_bindings.py`
- `62796f8440df6dedda7eca59895b7b0e5a895f41` â€” ACCEPTED for cluster validation. lua handoff: eight verification corrections, 25 static tests and unverified original callback dependencies
  Paths: `native/parallel/handoffs/lua.md`
- `c36954eb002f8440520445d7489e584921cca04d` â€” ACCEPTED for cluster validation. lua: recover six source-documented original launcher bindings without inventing symbolic vtable VAs
  Paths: `native/tools/map_original_lua_registrations.py`
- `e446143debf61ad286603909fc33eaa8b63d0e49` â€” ACCEPTED for cluster validation. lua tests: source-backed launcher registrations preserve unknown symbolic vtable addresses
  Paths: `native/tools/test_map_original_lua_registrations.py`
- `ee2595513a604c6737bebf703a9b5139e7bdc705` â€” ACCEPTED for cluster validation. lua: reconcile original launcher bindings together with published flag vtables and ELF calls
  Paths: `native/tools/reconcile_original_lua_bindings.py`
- `63acca0bb6b1139a96c9a417745475b0c710112f` â€” ACCEPTED for cluster validation. lua tests: preserve symbolic original launcher evidence separately from verified ELF calls
  Paths: `native/tools/test_reconcile_original_lua_bindings.py`
- `3499d8e73b414689519cc903e73b89e7fa74a527` â€” ACCEPTED for cluster validation. lua handoff: six original launcher registrations, symbolic VA handling and 27 passing Python tests
  Paths: `native/parallel/handoffs/lua.md`
- `b5a9079b98d6a24072255234fec064f9a8cad564` â€” ACCEPTED for cluster validation. lua: enable retail manager strictness only after host Lua 5.0 accepts a validated bytecode chunk
  Paths: `native/src/lua_stage.cpp`
- `0b70010439c7d890f9fef4076b9f9728b3c79b36` â€” ACCEPTED for cluster validation. lua tests: invalid Lua 5.0 opcode cannot poison subsequent source-only stage bindings
  Paths: `native/tests/retail_lua50_test.cpp`
- `a154009b09ae9c54391fa91f5f3bdcb08abb8d82` â€” ACCEPTED for cluster validation. lua: recover provenance-backed literal actor/controller names from original 5.0 direct CALL arguments
  Paths: `native/tools/audit_original_lua50_calls.py`
- `d89434167c6981d66263cb1605bfa6b26193970d` â€” ACCEPTED for cluster validation. lua tests: verify exact first-string arguments, MOVE aliases and no false literals after clobber/branch
  Paths: `native/tools/test_audit_original_lua50_calls.py`
- `80ed4f65716c30fb509a7d23ae2fa5791a572ec3` â€” ACCEPTED for cluster validation. lua: generate original stage actor/controller/flag dependency manifests from proven Lua 5.0 literals
  Paths: `native/tools/reconcile_original_lua_bindings.py`
- `13f9c38ee0209fc604cfaf228a32c38481f0f548` â€” ACCEPTED for cluster validation. lua tests: actor/controller per-stage manifest and literal evidence validation
  Paths: `native/tools/test_reconcile_original_lua_bindings.py`

Tests absent from baseline CMake (runtime conditional manifest also needs review): none

### input-audio

Tip `164120567c1c1565c2145e5a5524761e70a1edaf`; merge base `d483436416c14927851853f01c6ad66fe4cfbbf2`; 11 commits; 18 changed paths.

Decision: BLOCKED: outside selected cluster; needs review.

- `de3cf850826cf73965943444d0c814457fa7c57b` â€” BLOCKED: outside selected cluster; needs review. input-audio: add source-anchored pad and Lua sound dispatch boundaries
  Paths: `native/include/ae3/original_audio_runtime.hpp`, `native/include/ae3/original_pad_runtime.hpp`, `native/src/original_audio_runtime.cpp`, `native/src/original_pad_runtime.cpp`, `native/src/original_pad_win32.cpp`, `native/tests/original_input_audio_runtime_test.cpp`
- `3f27ce8bf35fa5147fbc2cb5fe402c7e5ceede93` â€” BLOCKED: outside selected cluster; needs review. input-audio: verify retail se and wait Lua defaults against original USA ELF
  Paths: `native/include/ae3/original_audio_runtime.hpp`, `native/src/controller_timeline.cpp`, `native/src/original_audio_runtime.cpp`, `native/tests/controller_timeline_test.cpp`, `native/tests/original_input_audio_runtime_test.cpp`, `native/tools/verify_original_input_audio_retail.py`
- `93896051a6c1a998fa64f343167ffdac17960641` â€” BLOCKED: outside selected cluster; needs review. input-audio: document original ELF evidence, tested native boundaries and integration recipe
  Paths: `native/parallel/handoffs/input-audio.md`
- `9e40e4a67575018e610456f5f8f2d9921393fd0c` â€” BLOCKED: outside selected cluster; needs review. input-audio: recover original retail pad polling, radial analog response and game input records
  Paths: `native/include/ae3/original_retail_pad.hpp`, `native/src/original_retail_pad.cpp`, `native/tests/original_retail_pad_test.cpp`, `native/tools/verify_original_retail_pad.py`
- `55449fcbe800b59c557d6d9abd88ce8e27af821f` â€” BLOCKED: outside selected cluster; needs review. input-audio: document retail input recovery and original game controller state integration
  Paths: `native/parallel/handoffs/input-audio.md`
- `17ff9527ccda5a721628359018a13e9b8b1cca56` â€” BLOCKED: outside selected cluster; needs review. input-audio: recover original SE constructors, allocation sizes and entity-sound Lua fields
  Paths: `native/include/ae3/original_audio_runtime.hpp`, `native/src/original_audio_runtime.cpp`, `native/tests/original_input_audio_runtime_test.cpp`, `native/tools/verify_original_input_audio_retail.py`
- `0017f41404564cf2ec7ebdac4f7aa4b111493f21` â€” BLOCKED: outside selected cluster; needs review. input-audio: append recovered retail sound command constructors and integration limits
  Paths: `native/parallel/handoffs/input-audio.md`
- `1e97fbbca81b640e9e8ade4c94278d5c6acfa917` â€” BLOCKED: outside selected cluster; needs review. input-audio: bridge native physical pads to source-verified retail channel reader
  Paths: `native/include/ae3/original_retail_pad.hpp`, `native/src/original_retail_pad.cpp`, `native/tests/original_retail_pad_test.cpp`
- `2c53f7223746a3ab3afa13a761624742387c757a` â€” BLOCKED: outside selected cluster; needs review. input-audio: document physical-to-retail pad channel bridge and tested integration
  Paths: `native/parallel/handoffs/input-audio.md`
- `21f0b09b50d9fafc86e80bcfb40552757d481e5a` â€” BLOCKED: outside selected cluster; needs review. input-audio: recover retail direct pad packet, independent two-port state and actuator output
  Paths: `native/include/ae3/original_pad_actuator.hpp`, `native/include/ae3/original_retail_pad.hpp`, `native/src/original_pad_actuator.cpp`, `native/src/original_retail_pad.cpp`, `native/tests/original_pad_actuator_test.cpp`, `native/tests/original_retail_pad_test.cpp`, `native/tools/verify_original_pad_actuator.py`
- `164120567c1c1565c2145e5a5524761e70a1edaf` â€” BLOCKED: outside selected cluster; needs review. input-audio: hand off retail six-byte and two-port input plus verified actuator chain and CMake recipe
  Paths: `native/parallel/handoffs/input-audio.md`

Tests absent from baseline CMake (runtime conditional manifest also needs review): `native/tests/original_input_audio_runtime_test.cpp`, `native/tests/original_pad_actuator_test.cpp`, `native/tests/original_retail_pad_test.cpp`
