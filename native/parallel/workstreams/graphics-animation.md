# Workstream: Original Graphics, Models & Animation

**Dedicated branch:** `parallel/graphics-animation`
**Integration branch:** `native/original-elf-engine-recovery-20261020`
**Priority:** Critical — render actual original world geometry and animated original actors

## Exclusive code ownership

native/src/level_scene.cpp, gltf_scene.cpp, i3m_animation.cpp, native_skinning.cpp, glb_scene_probe.cpp, level_scene_probe.cpp; original scene/camera/material/render adapters created within graphics-owned files; matching headers/tests

## Your scope for the next substantial passes

1. Recover original stage mesh hierarchy, I3M model skeletons, animation tracks, skinning behavior and material/texture assignments.
2. Trace source rendering/animation function call chains and determine correct frame, transform and coordinate conventions.
3. Produce native renderer-facing scene and animation APIs which consume genuine original assets and can be called by the Windows integrator.
4. Run tests against the user's private original stage/model resources when available, explicitly report rendering not yet booted.

## Starting evidence and blockers

Native scene and skinning modules exist, but full original game drawing and actor animation have not been verified end-to-end.

## Strict boundaries

Do not change the Windows app entry point app_win32.cpp or native/CMakeLists.txt; do not modify raw asset parsers owned by assets.
Review `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md` and `native/parallel/INTEGRATION_PROTOCOL.md`. Do not edit global CMake/build scripts, shared docs, other teams' source, or master/default main. Keep original ELF/ISO/Lua/resource bytes private. **Do not launch GitHub Actions**. No emulation, replacement engine, or fake gameplay.

## Kickoff prompt — copy into your own ChatGPT conversation

> Continue the authentic original Ape Escape 3 USA native Windows C/C++ decompilation. Repository: https://github.com/Ukkitan0/Getchu-3-PC. My **exclusive** subsystem is **Original Graphics, Models & Animation**. Work **only** on GitHub branch `parallel/graphics-animation`, which already contains the common native recovery source and parallel collaboration instructions. Read `native/AGENTS.md`, `native/parallel/WORKSTREAMS.md`, and `native/parallel/workstreams/graphics-animation.md` before changing code. Recover the original game engine from privately owned `SCUS_975.01`, not an emulator, demo or replacement gameplay engine. Make the largest practical source-backed subsystem recovery per reply; actually implement and test C/C++ code, commit source to your dedicated branch, and append exact tests, recovered VAs, dependencies and handoff instructions to `native/parallel/handoffs/graphics-animation.md`. Do not edit `native/CMakeLists.txt` or another category's code; write a CMake change request into the handoff for the integrator. Never commit game assets or manually trigger GitHub Actions. Do not claim Windows game boot without running the actual native game. Begin by inspecting your owned modules and dependencies and make maximum useful verified progress.
