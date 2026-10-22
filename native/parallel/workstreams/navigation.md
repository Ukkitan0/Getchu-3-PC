# Workstream: Original Navigation & Pathfinding

**Dedicated branch:** `parallel/navigation`
**Integration branch:** `native/original-elf-engine-recovery-20261020`
**Priority:** High — complete original route and path owner lifecycle

## Exclusive code ownership

native/src/original_nav*, original_navrange_*, original_actor_nav_controller.cpp, original_actor_nav_route.cpp, original_actor_route_follower.cpp, original_actor_type2_target.cpp, original_spatial_index.cpp, original_spatial_support.cpp; headers/tests/tools

## Your scope for the next substantial passes

1. Use existing original spatial/path graph code and audit real source callers to find unresolved route owner dispatch.
2. Finish original route decisions, neighbor selection, type-1/2 action links, route lifetime and stage graph synchronization.
3. Prove source behavior against original stage navigation PCK where privately available, not synthetic routes alone.
4. Expose APIs consumed by the VU0/actor pipeline, leaving ownership of the latter unchanged.

## Starting evidence and blockers

Original nav route/search and actor route-following have substantial earlier recoveries; coverage is not the same as connected native gameplay.

## Strict boundaries

Do not modify original_actor_navigation_pipeline.* or original_polygon_* (VU0), or original_stage_spatial_loader.* (assets).
Review `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md` and `native/parallel/INTEGRATION_PROTOCOL.md`. Do not edit global CMake/build scripts, shared docs, other teams' source, or master/default main. Keep original ELF/ISO/Lua/resource bytes private. **Do not launch GitHub Actions**. No emulation, replacement engine, or fake gameplay.

## Kickoff prompt — copy into your own ChatGPT conversation

> Continue the authentic original Ape Escape 3 USA native Windows C/C++ decompilation. Repository: https://github.com/Ukkitan0/Getchu-3-PC. My **exclusive** subsystem is **Original Navigation & Pathfinding**. Work **only** on GitHub branch `parallel/navigation`, which already contains the common native recovery source and parallel collaboration instructions. Read `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md`, and `native/parallel/workstreams/navigation.md` before changing code. Recover the original game engine from privately owned `SCUS_975.01`, not an emulator, demo or replacement gameplay engine. Make the largest practical source-backed subsystem recovery per reply; actually implement and test C/C++ code, commit source to your dedicated branch, and append exact tests, recovered VAs, dependencies and handoff instructions to `native/parallel/handoffs/navigation.md`. Do not edit `native/CMakeLists.txt` or another category's code; write a CMake change request into the handoff for the integrator. Never commit game assets or manually trigger GitHub Actions. Do not claim Windows game boot without running the actual native game. Begin by inspecting your owned modules and dependencies and make maximum useful verified progress.
