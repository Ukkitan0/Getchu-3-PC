# Workstream: Input, Gadgets & Original Audio Runtime

**Dedicated branch:** `parallel/input-audio`
**Integration branch:** `native/original-elf-engine-recovery-20261020`
**Priority:** High — map original controls/gadgets and sound behavior onto native Windows interfaces

## Exclusive code ownership

native/src/controller_timeline.cpp, native_walking.cpp, native_walking_probe.cpp; new isolated original input/controller and audio runtime adapter files; matching headers/tests

## Your scope for the next substantial passes

1. Recover original pad state, analog/button conversions, gadget activation and input event timing from SCUS_975.01.
2. Build a native Windows keyboard/controller translation layer preserving original action semantics without driving replacement locomotion.
3. Recover sound/music triggering and sequencing from original game API evidence, using the asset team's original audio descriptor parsing.
4. Provide clean native input and audio callback interfaces for the gameplay and Windows integration chats.

## Starting evidence and blockers

Some controller timeline and host walking modules exist; source input/audio integration into real original actor/game update remains incomplete.

## Strict boundaries

Do not edit the asset-owned original_audio_descriptor parser, gameplay action source, or app_win32.cpp/CMake.
Review `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md` and `native/parallel/INTEGRATION_PROTOCOL.md`. Do not edit global CMake/build scripts, shared docs, other teams' source, or master/default main. Keep original ELF/ISO/Lua/resource bytes private. **Do not launch GitHub Actions**. No emulation, replacement engine, or fake gameplay.

## Kickoff prompt — copy into your own ChatGPT conversation

> Continue the authentic original Ape Escape 3 USA native Windows C/C++ decompilation. Repository: https://github.com/Ukkitan0/Getchu-3-PC. My **exclusive** subsystem is **Input, Gadgets & Original Audio Runtime**. Work **only** on GitHub branch `parallel/input-audio`, which already contains the common native recovery source and parallel collaboration instructions. Read `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md`, and `native/parallel/workstreams/input-audio.md` before changing code. Recover the original game engine from privately owned `SCUS_975.01`, not an emulator, demo or replacement gameplay engine. Make the largest practical source-backed subsystem recovery per reply; actually implement and test C/C++ code, commit source to your dedicated branch, and append exact tests, recovered VAs, dependencies and handoff instructions to `native/parallel/handoffs/input-audio.md`. Do not edit `native/CMakeLists.txt` or another category's code; write a CMake change request into the handoff for the integrator. Never commit game assets or manually trigger GitHub Actions. Do not claim Windows game boot without running the actual native game. Begin by inspecting your owned modules and dependencies and make maximum useful verified progress.
