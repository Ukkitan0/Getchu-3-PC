# Workstream: Original Lua 5.0 Scripting & Game Logic Bindings

**Dedicated branch:** `parallel/lua`
**Integration branch:** `native/original-elf-engine-recovery-20261020`
**Priority:** Critical — execute original stage Lua and game callbacks in native host runtime

## Exclusive code ownership

native/src/retail_lua50.cpp, lua_stage.cpp, lua_stage_runner.cpp, stage_scripts.cpp; original Lua recovery, registration and stage callback investigation tools; matching headers/tests

## Your scope for the next substantial passes

1. Audit authentic original Lua 5.0 bytecode and script binding tables extracted privately from source game assets.
2. Recover the original Lua-to-C++ binding contracts for actor managers, stages, missions, gadget interactions and event dispatch.
3. Make original stage scripts initialize and drive their genuine original callbacks through native host Lua 5.0 APIs.
4. Provide precise integration interfaces to gameplay/asset owners, documenting any unimplemented original functions.

## Starting evidence and blockers

An embedded Lua 5.0.3 host library and existing stage scripting modules exist; actual complete stage script execution with original engine bindings is not yet proven.

## Strict boundaries

Do not implement invented mission scripts, edit actor gameplay source or alter native/CMakeLists.txt.
Review `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md` and `native/parallel/INTEGRATION_PROTOCOL.md`. Do not edit global CMake/build scripts, shared docs, other teams' source, or master/default main. Keep original ELF/ISO/Lua/resource bytes private. **Do not launch GitHub Actions**. No emulation, replacement engine, or fake gameplay.

## Kickoff prompt — copy into your own ChatGPT conversation

> Continue the authentic original Ape Escape 3 USA native Windows C/C++ decompilation. Repository: https://github.com/Ukkitan0/Getchu-3-PC. My **exclusive** subsystem is **Original Lua 5.0 Scripting & Game Logic Bindings**. Work **only** on GitHub branch `parallel/lua`, which already contains the common native recovery source and parallel collaboration instructions. Read `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md`, and `native/parallel/workstreams/lua.md` before changing code. Recover the original game engine from privately owned `SCUS_975.01`, not an emulator, demo or replacement gameplay engine. Make the largest practical source-backed subsystem recovery per reply; actually implement and test C/C++ code, commit source to your dedicated branch, and append exact tests, recovered VAs, dependencies and handoff instructions to `native/parallel/handoffs/lua.md`. Do not edit `native/CMakeLists.txt` or another category's code; write a CMake change request into the handoff for the integrator. Never commit game assets or manually trigger GitHub Actions. Do not claim Windows game boot without running the actual native game. Begin by inspecting your owned modules and dependencies and make maximum useful verified progress.
