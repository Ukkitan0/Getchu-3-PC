# Workstream: Original VU0 Mathematics, Collision & Movement

**Dedicated branch:** `parallel/vu0-physics`
**Integration branch:** `native/original-elf-engine-recovery-20261020`
**Priority:** Critical — source-accurate actor vector transforms and polygon interactions

## Exclusive code ownership

native/src/original_vu0_geometry.cpp, original_actor_fallback_*, original_actor_random_triangle.cpp, original_actor_ray_geometry.cpp, original_actor_target_dispatch.cpp, original_actor_navigation_pipeline.cpp, original_stage_actor_navigation.cpp, original_polygon_*, original_stage_polygon_raycast.cpp, i3c_collision.cpp, original_game_random.cpp; matching headers/tests

## Your scope for the next substantial passes

1. Finish exact original 0x57CB4C–0x57CBB8 and 0x57CC48–0x57CCB0 VU0 candidate transform mathematics.
2. Preserve original source actor-owner 16-byte vector, controller+0x20 vector, global VU0 Q state and independent VU R plus EE MT19937 state.
3. Connect remaining collision and actor movement calculations to the original native navigation/actor interfaces.
4. Test source bit patterns, VU operation order, original 59 polygon resource data and actual connected movement chain.

## Starting evidence and blockers

At master 879c347, fallback selection exists, but VU0 candidate transforms and full actor movement loop remain incomplete.

## Strict boundaries

Do not edit original_actor_nav_* (navigation), original_stage_spatial_loader.* (assets), app_win32.cpp or CMake.
Review `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md` and `native/parallel/INTEGRATION_PROTOCOL.md`. Do not edit global CMake/build scripts, shared docs, other teams' source, or master/default main. Keep original ELF/ISO/Lua/resource bytes private. **Do not launch GitHub Actions**. No emulation, replacement engine, or fake gameplay.

## Kickoff prompt — copy into your own ChatGPT conversation

> Continue the authentic original Ape Escape 3 USA native Windows C/C++ decompilation. Repository: https://github.com/Ukkitan0/Getchu-3-PC. My **exclusive** subsystem is **Original VU0 Mathematics, Collision & Movement**. Work **only** on GitHub branch `parallel/vu0-physics`, which already contains the common native recovery source and parallel collaboration instructions. Read `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md`, and `native/parallel/workstreams/vu0-physics.md` before changing code. Recover the original game engine from privately owned `SCUS_975.01`, not an emulator, demo or replacement gameplay engine. Make the largest practical source-backed subsystem recovery per reply; actually implement and test C/C++ code, commit source to your dedicated branch, and append exact tests, recovered VAs, dependencies and handoff instructions to `native/parallel/handoffs/vu0-physics.md`. Do not edit `native/CMakeLists.txt` or another category's code; write a CMake change request into the handoff for the integrator. Never commit game assets or manually trigger GitHub Actions. Do not claim Windows game boot without running the actual native game. Begin by inspecting your owned modules and dependencies and make maximum useful verified progress.
