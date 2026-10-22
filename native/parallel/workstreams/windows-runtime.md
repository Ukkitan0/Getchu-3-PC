# Workstream: Native Windows Runtime & Integration

**Dedicated branch:** `parallel/windows-runtime`
**Integration branch:** `native/original-elf-engine-recovery-20261020`
**Priority:** Critical — deliver a real, buildable and bootable original native game executable

## Exclusive code ownership

native/src/app_win32.cpp; native/CMakeLists.txt; native Windows bootstrap/build scripts; glue and integration tests; shared build-target ownership

## Your scope for the next substantial passes

1. Audit the full native CMake project from a clean checkout and repair compile/link errors without emulation.
2. Map original USA executable process/main-loop initialization to native Windows and connect stage, Lua, actor, graphics and input services.
3. Implement native real-game boot checkpoints with observable logs and actual original stage content.
4. Integrate specialist handoffs selectively, keeping APIs and all CMake registration in one owner.

## Starting evidence and blockers

The current master branch has original-subsystem tests but no verified full native Windows game boot. Lua/cgltf FetchContent may require network.

## Strict boundaries

Do not invent a substitute demo scene and call it the game. Do not rewrite other teams' engine logic; integrate their reviewed changes.
Review `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md` and `native/parallel/INTEGRATION_PROTOCOL.md`. Do not edit global CMake/build scripts, shared docs, other teams' source, or master/default main. Keep original ELF/ISO/Lua/resource bytes private. **Do not launch GitHub Actions**. No emulation, replacement engine, or fake gameplay.

## Kickoff prompt — copy into your own ChatGPT conversation

> Continue the authentic original Ape Escape 3 USA native Windows C/C++ decompilation. Repository: https://github.com/Ukkitan0/Getchu-3-PC. My **exclusive** subsystem is **Native Windows Runtime & Integration**. Work **only** on GitHub branch `parallel/windows-runtime`, which already contains the common native recovery source and parallel collaboration instructions. Read `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md`, and `native/parallel/workstreams/windows-runtime.md` before changing code. Recover the original game engine from privately owned `SCUS_975.01`, not an emulator, demo or replacement gameplay engine. Make the largest practical source-backed subsystem recovery per reply; actually implement and test C/C++ code, commit source to your dedicated branch, and append exact tests, recovered VAs, dependencies and handoff instructions to `native/parallel/handoffs/windows-runtime.md`. Do not edit `native/CMakeLists.txt` or another category's code; write a CMake change request into the handoff for the integrator. Never commit game assets or manually trigger GitHub Actions. Do not claim Windows game boot without running the actual native game. Begin by inspecting your owned modules and dependencies and make maximum useful verified progress.
