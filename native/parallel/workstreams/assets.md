# Workstream: Original Game Resources & Asset Formats

**Dedicated branch:** `parallel/assets`
**Integration branch:** `native/original-elf-engine-recovery-20261020`
**Priority:** Critical — load original game data end to end without substitute content

## Exclusive code ownership

native/src/archive.cpp, pck_archive.cpp, tim2.cpp, original_stage_spatial_loader.cpp, original_spatial_raw_bridge.cpp; original_resource_*, original_attribute_*, original EXDB and original_audio_descriptor parsing; ISO/VFI/PCK extraction tools; matching headers/tests

## Your scope for the next substantial passes

1. Map original game archive and resource owner lifecycles to native loaders using original files privately supplied by the owner.
2. Validate navigation and stage geometry resource tables, mesh and texture resource references and binary offsets.
3. Recover original EXDB/resource lookup, dictionary/attribute loading, and original descriptor interpretation.
4. Expose stable resource-loader APIs for graphics/Lua/gameplay/integration; record actual source/test provenance.

## Starting evidence and blockers

There are many existing original asset parsers and audits, but the complete genuine stage-to-native game resource pipeline is not confirmed.

## Strict boundaries

Do not modify rendering/model animation, actor physics, game logic or global build configuration.
Review `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md` and `native/parallel/INTEGRATION_PROTOCOL.md`. Do not edit global CMake/build scripts, shared docs, other teams' source, or master/default main. Keep original ELF/ISO/Lua/resource bytes private. **Do not launch GitHub Actions**. No emulation, replacement engine, or fake gameplay.

## Kickoff prompt — copy into your own ChatGPT conversation

> Continue the authentic original Ape Escape 3 USA native Windows C/C++ decompilation. Repository: https://github.com/Ukkitan0/Getchu-3-PC. My **exclusive** subsystem is **Original Game Resources & Asset Formats**. Work **only** on GitHub branch `parallel/assets`, which already contains the common native recovery source and parallel collaboration instructions. Read `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md`, and `native/parallel/workstreams/assets.md` before changing code. Recover the original game engine from privately owned `SCUS_975.01`, not an emulator, demo or replacement gameplay engine. Make the largest practical source-backed subsystem recovery per reply; actually implement and test C/C++ code, commit source to your dedicated branch, and append exact tests, recovered VAs, dependencies and handoff instructions to `native/parallel/handoffs/assets.md`. Do not edit `native/CMakeLists.txt` or another category's code; write a CMake change request into the handoff for the integrator. Never commit game assets or manually trigger GitHub Actions. Do not claim Windows game boot without running the actual native game. Begin by inspecting your owned modules and dependencies and make maximum useful verified progress.
