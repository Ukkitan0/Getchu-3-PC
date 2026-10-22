# Native original-engine recovery pass — 2026-10-22

This is a verified engineering checkpoint, not a complete decompilation or playable game.

## New original-code recovery

Implemented native `originalManagerInitialize_57bcb8` from the owner's authentic `SCUS_975.01`, span `0x57BCB8..0x57BF58` inclusive, 169 original words. Full source-span SHA-256: `90ced0cff1cd0ad2f9fa260859db156e1dcc6fb4c4c295dfe0d62fe0df6a9e0d`.

The existing manager constructor at `0x57BA98/0x57BAF8` previously delegated this initializer to an abstract unresolved call. The new implementation recovers its branch control flow, near-owner guard, original current-actor action-kind query, second-route-polygon selection, target initialization, general projection operands/midpoint, and ordered method-pair writes. It reuses existing native navigation, source vertex access, original RNG/triangle weighting and XYZ-dot routines. It does not re-decompile those helpers or overwrite specialist work.

Source-defined operations:

- Alternative-mode guard: controller+0x20 minus owner+0x14->0x40; XYZ dot `0x4748AC`, square root, ordered strict comparison against original `0x708A20 = 2.0f`. Early return preserves the source's low-64-bit `$a1` writes to +8/+0xC.
- Multiple-polygon route path: `0x221104` obtains the route edge slot; `0x21B444` reads the current actor polygon's action kind. Kind 2 selects the second route polygon through existing `0x57BDB8` translation and reads its three original vertices through `0x21B458`.
- Type-2 target: original two RNG calls `0x36F4D4`/`0x36F548`, scale bits `0x3F4CCCCD`, inset bits `0x3D888888`, three ordered multiplies and two adds. Existing `0x57C98C` computes the same verified sequence. Original method pair becomes `(0x57C554,0)` and target vector is stored at +0x40.
- General path: original direction*25 plus controller point; explicit original `0x21B854` projection dependency; four-lane midpoint `(controller + projected)*0.5` stored at +0x60; explicit actor-owner binding for recovered `0x57C7CC`; only after callback completion store method pair `(0x57BF5C,0)`.

No projection or owner is fabricated. This routine cannot make the original game boot until its real owners and remaining dependencies are connected.

## Integration and preservation

Native baseline: `d483436416c14927851853f01c6ad66fe4cfbbf2` on `native/original-elf-engine-recovery-20261020`. Working branch: `codex/integration-20261022`.

Eight specialist tips inventoried, 377 commits and all exact changed paths recorded in `INTEGRATION_LEDGER_20261022.md`. No changed-path overlaps. Runtime, assets, Lua and gameplay were reviewed as a selected cluster and merged separately, preserving authorship/history. Navigation, VU0, graphics and input/audio remain pending review. Main, integration baseline and all eight specialist branches are preserved; no force push, deletion or Actions dispatch.

The selected source changes now compile together in the actual Windows project. Added missing assets CMake/test registrations, connected stage registry ownership to the Lua audit, fixed source type-before-name selection, tightened missing retail manager errors and typed Lua transport, and repaired proven test fixture/Windows compilation defects.

## Inputs and reproducibility

Private original inputs were validated before analysis:

| Input | SHA-256 |
|---|---|
| Original USA ELF | `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63` |
| Full original ISO, 2,174,091,264 bytes | `5b4c6272c9daf190b3f72081f124edd7ea2af3cc0dafeaef206ded97a022fd09` |
| Private source ZIP | `1382368ee06e8025f2d6de54a6f1b6bc755bc20bb523712670ffff7625edf7e3` |

Private sources remain under ignored `native/private/codex_retail/`; ISO remains at the supplied owner path. No original binary/resource/script bytes are included in source deliverables.

Build: CMake Visual Studio 17 2022 generator, x64, Release; actual MSVC compiler and D3D11 Windows target. Configure `AE3_RETAIL_USA_ELF`, `AE3_RETAIL_USA_ISO`, `AE3_RETAIL_WOODS_C_FOLDER`, `AE3_RETAIL_WOODS_C_NAVIGATION_PCK`, `AE3_RETAIL_WOODS_C_EXDB_PCK`, and `AE3_RETAIL_LUA_CORPUS` to owner-private inputs.

```powershell
cmake -S native -B native/build -G "Visual Studio 17 2022" -A x64
cmake --build native/build --config Release --parallel 8
ctest --test-dir native/build -C Release --output-on-failure -j 8
python native/tools/verify_original_manager_initializer.py <private-SCUS_975.01>
```

Full build succeeded, including `Getchu3Native.exe`. Full CTest: **143 passed, 0 failed, 1 blocked/skipped**, 144 registered. This includes initializer focused tests, all **15 authentic woods_c type-2 actions**, complete source-provenance audit, selected source regressions, five original PCK packages (160 members), original navigation/EXDB checks and Lua bridge tests. New behavior tests were observed failing before implementation/fixes. Builds emit existing MSVC/third-party warnings; this is not a warning-free claim.

## Original Lua evidence and limits

All **168** handoff Lua chunks match their actual ISO registry payload bytes. Structural retargeting retains **135,820 original opcodes** across **805 original functions**. Top-level execution succeeds for **88** chunks; **77** stop at unrecovered `lua_param` manager semantics, and **3** boss chunks stop at missing `enlarge_se_range`. No `setupArea` invocation or gameplay is implied. The execution check reports blocked code 77 only for these explicitly identified dependencies; unexpected VM/transport/other failures fail normally.

The original binding reconciliation also identifies missing launcher functions and `LocaleInfo_getID`; registration evidence is not a callback implementation. The full per-stage dependency report stays private.

## Actual Win32 startup

Launched the actual native Windows executable with the verified ISO and ELF. Observed process exit **4** and checkpoint log:

```text
WINDOWS_WINDOW: PASS
D3D11_DEVICE: PASS
RETAIL_DISC: PASS
RETAIL_STATIC_CONSTRUCTORS: BLOCKED
First unported original static constructor VA 0x38AA18; 6 of 143 original constructors executed
```

This confirms a real Windows launch to an authentic recovery boundary, not gameplay. The new actor initializer is not yet reached by that startup path.

## Next original-engine dependencies

1. Startup #7: `0x38AA18 -> 0x38A978`, allocator `0x44E420 -> 0x101584`, failure path `0x44E280`, original 0x18-byte list owner. Do not substitute an arbitrary heap sentinel.
2. Connect the new `0x57BCB8` translation to genuine actor/route ownership; recover/validate `0x21B854` and the original incoming operands for `0x57C7CC`.
3. Recover original Player/Monkey scheduler, actor lifetimes and vtable update callers; original state/action/physics/render/input integration remains incomplete.
4. Recover original `lua_param`, live entity and stage bindings, including `enlarge_se_range`, with authentic source VAs/semantics; preserve original Lua.
5. Review pending specialist histories and connect real native rendering, input/audio, stage selection and progression. No playability claim until the original game runs its real title-to-stage-to-capture/progression loop.
