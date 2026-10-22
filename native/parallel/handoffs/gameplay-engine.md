# Original Actors, Gameplay & AI — specialist handoff

**Specialist branch:** `parallel/gameplay-engine`  
**Integration target:** `native/original-elf-engine-recovery-20261020`  
**Starting commit:** `d483436416c14927851853f01c6ad66fe4cfbbf2`

## Delivered source / commits (2026-10-21 America/New_York)

- `165b48f92b545227178aa9e3154a87acd2aaae71` — `native/src/actor_world.cpp`: the retail-script actor binder now **rejects** unresolved original spawn positions. It no longer constructs/render-schedules game actors at invented origin (0,0,0) if there is neither an original Lua coordinate nor one unambiguous matching original PLC tag; increments both `unresolvedSpawnPositions` and `unboundEntities` with explicit diagnostics.
- `47851309027cb3c9d35909fd3042b8711cf52e8e` — `native/include/ae3/actor_world.hpp`: documents tightened actor placement contract.
- `17f276a1eb73e369005a437a83405188dbfa44df` — `native/tests/actor_world_test.cpp`: adds regression cases for missing explicit position and absent retail PLC tag; preserves existing real-tag placement, multiple independent actor, and asset-binding tests. The tests are source changes and have **not been executed** in a full compiled build.

This is a correctness/safety repair of an **original-resource adapter**, not a reconstructed original gameplay or AI loop.

## Original executable / resource provenance

Original `SCUS_975.01` SHA-256 expected `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`, owner-private and unavailable in this turn. Existing source-level call chains give Monkey `0x571904` -> `0x310890`, `0x571E10`, `0x316174`, optional `0x3108D4` or `0x20AF68` and deletion; Player `0x34A57C` secondary mode dispatch; manager `0x57BA00` and child `0x57BCB8`. Full original Player/Monkey update scheduling, actor ownership and real VU0-mediated character movement are still missing. No new retail VA recovery is claimed here, and no commercial data was committed.

## Exact tests and runtime status

- 11/11 static source-contract checks across both parallel branches passed, including both new unresolved-spawn test assertions and updated binder diagnostic.
- The integration CMake manifest has 154 source/test path references present in the base tree; this does not prove build correctness.
- **Not compiled/run:** `actor_world_test.cpp`, source/ELF audits, CTest, Windows executable, original gameplay. No compiler-backed validation in this turn; network clone and the private source ELF were unavailable. No GitHub Actions triggered.

## Integration request for Windows Runtime

No `native/CMakeLists.txt` change needed: `src/actor_world.cpp` and `tests/actor_world_test.cpp` are already registered. Merge only the three gameplay-owned files after review. The renderer must tolerate fewer actors: `report.unresolvedSpawnPositions` means **actors rejected**, not game entities instantiated at the origin. Do not substitute missing game managers with native walking tests.

## Remaining connected original-engine recovery

1. Extract verified `0x57BCB8` complete original MIPS/EE routine and its incoming initialization/ownership dependencies from the private USA ELF; port to native C++ with testable explicit backing APIs.
2. Trace original actor scheduler into both Player and Monkey update callchains (including `0x571904` and child virtual slot 5); implement call order and lifecycle only after opcode verification.
3. Wire recovered action, attachment, collision/nav and Lua engines into the genuine scheduler; report unresolved original vtable dispatch rather than fabricating AI/behaviors.
4. Run native component tests and then real Windows game boot with owner-provided data; no playability claim until genuine gameplay executes.
