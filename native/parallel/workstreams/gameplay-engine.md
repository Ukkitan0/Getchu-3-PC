# Workstream: Original Actors, Gameplay & AI

**Dedicated branch:** `parallel/gameplay-engine`
**Integration branch:** `native/original-elf-engine-recovery-20261020`
**Priority:** Critical — original actor life cycle, player/mob state and frame updates

## Exclusive code ownership

native/src/actor_world.cpp, original_action_runtime.cpp, original_player_*, original_manager_child.cpp, original_monkey_accessors.cpp, original_attachment_*, player_state.cpp; matching headers and tests

## Your scope for the next substantial passes

1. Trace the real game actor/world update callers in original SCUS_975.01, including original player and monkey controllers.
2. Recover manager-owned actor creation, deletion, update scheduling, animation/action state and collision hook contracts.
3. Connect actual Lua callbacks and player/monkey actor state through explicit interfaces without writing Lua-owned files.
4. Expose a stable actor tick interface and source-backed tests for the integration owner.

## Starting evidence and blockers

Navigation and VU0 logic are partly recovered; actor tick/update and genuine game-world integration remain unfinished.

## Strict boundaries

Do not edit original_actor_navigation_pipeline.* (owned by VU0) or original_actor_nav_* (navigation); propose interfaces in your handoff.
Review `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md` and `native/parallel/INTEGRATION_PROTOCOL.md`. Do not edit global CMake/build scripts, shared docs, other teams' source, or master/default main. Keep original ELF/ISO/Lua/resource bytes private. **Do not launch GitHub Actions**. No emulation, replacement engine, or fake gameplay.

## Kickoff prompt — copy into your own ChatGPT conversation

> Continue the authentic original Ape Escape 3 USA native Windows C/C++ decompilation. Repository: https://github.com/Ukkitan0/Getchu-3-PC. My **exclusive** subsystem is **Original Actors, Gameplay & AI**. Work **only** on GitHub branch `parallel/gameplay-engine`, which already contains the common native recovery source and parallel collaboration instructions. Read `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md`, and `native/parallel/workstreams/gameplay-engine.md` before changing code. Recover the original game engine from privately owned `SCUS_975.01`, not an emulator, demo or replacement gameplay engine. Make the largest practical source-backed subsystem recovery per reply; actually implement and test C/C++ code, commit source to your dedicated branch, and append exact tests, recovered VAs, dependencies and handoff instructions to `native/parallel/handoffs/gameplay-engine.md`. Do not edit `native/CMakeLists.txt` or another category's code; write a CMake change request into the handoff for the integrator. Never commit game assets or manually trigger GitHub Actions. Do not claim Windows game boot without running the actual native game. Begin by inspecting your owned modules and dependencies and make maximum useful verified progress.
