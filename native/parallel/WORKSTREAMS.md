# Exclusive subsystem ownership — Ape Escape 3 native original port

The goal is to recover **the authentic original game and engine** as one native Windows executable. Overlapping read access is expected; overlapping **writes** are not.

| Team / dedicated branch | Owns these source/headers/tests (and matching docs/tools) | Must NOT edit |
|---|---|---|
| **windows-runtime** / `parallel/windows-runtime` | `native/src/app_win32.cpp`, `native/CMakeLists.txt`, Windows bootstrap and build/launch scripts, integration harness, repository workflow configuration (but never dispatch) | Gameplay/math/graphics/asset/Lua implementation source |
| **gameplay-engine** / `parallel/gameplay-engine` | `actor_world.*`, `original_player_*.*`, `original_action_runtime.*`, `original_manager_child.*`, `original_monkey_accessors.*`, `original_attachment_*.*`, `player_state.*`; matching tests | `app_win32.cpp`, nav/math/graphics, CMake |
| **navigation** / `parallel/navigation` | `original_nav*.*` (including navrange and navgraph), `original_actor_nav_*.*`, `original_actor_route_follower.*`, `original_actor_type2_target.*`, `original_spatial_index.*`, `original_spatial_support.*`, route/search ownership, matching tests | VU/collision/actor *pipeline* implementation, CMake |
| **vu0-physics** / `parallel/vu0-physics` | `original_vu0_geometry.*`, `original_actor_fallback_*.*`, `original_actor_random_triangle.*`, `original_actor_ray_geometry.*`, `original_actor_target_dispatch.*`, `original_actor_navigation_pipeline.*`, `original_stage_actor_navigation.*`, `original_polygon_*.*`, `original_stage_polygon_raycast.*`, `i3c_collision.*`, `original_game_random.*`; matching tests | Base route search, stage format loader and parser, CMake |
| **graphics-animation** / `parallel/graphics-animation` | `level_scene.*`, `gltf_scene.*`, `i3m_animation.*`, `native_skinning.*`, `glb_scene_probe.*`, scene and model rendering logic, graphic tests | `app_win32.cpp`, raw asset archive parsers, CMake |
| **assets** / `parallel/assets` | `archive.*`, `pck_archive.*`, `tim2.*`, `original_stage_spatial_loader.*`, `original_spatial_raw_bridge.*`, `original_resource_*.*`, `original_attribute_*.*`, original EXDB/audio descriptor *parsing*, ISO/VFI/PCK extraction and file format tools | Actor physics and graphics rendering implementation, CMake |
| **lua** / `parallel/lua` | `retail_lua50.*`, `lua_stage.*`, `lua_stage_runner.*`, `stage_scripts.*`, retail Lua recovery tools and registration/binding tests | Gameplay system ownership; CMake |
| **input-audio** / `parallel/input-audio` | `controller_timeline.*`, `native_walking.*`, `native_walking_probe.*`, new independent native input/gadget/audio runtime adapters, and their tests | Parser `original_audio_descriptor` (assets), original actor game logic, CMake |

**Important precedence:** explicit names in this table take precedence over broad prefixes. In particular, `original_actor_navigation_pipeline.*` is **VU0/physics**, `original_actor_nav_controller.*` is **navigation**, and `original_stage_spatial_loader.*` is **assets**. The `original_audio_descriptor.*` **parser** belongs to assets; the sound runtime belongs to input/audio.

**Shared read-only interfaces:** `native/include/ae3/` header files owned by another category; `native/third_party/`; original ELF/retail audits; `native/CMakeLists.txt`. To change one, request it via your team's handoff; only the Windows/runtime integration owner reconciles the change.

**Native core only:** The repository root contains legacy research and emulation-era checkpoint files that are **not** the native game. Specialist work must remain in `native/` unless the integration owner coordinates a necessary native bootstrap change. Never silently resume EE/IOP emulation projects.

Each specialist may update its uniquely named `native/parallel/handoffs/<team>.md`; only the integrator edits the shared README, WORKSTREAMS, INTEGRATION_PROTOCOL, and AGENTS files. Each category brief is fixed at split; propose changes through the handoff.

## Priority and blocking dependencies

1. Runtime/integration establishes real executable boot, full build and stage activation.
2. Assets + Lua restore actual game stage loading and original script execution.
3. Gameplay + graphics recover actor updates, animation and rendering.
4. Navigation and VU0 complete authentic movement and path interactions.
5. Input/audio connects controls and sound to the original runtime.

Prioritize whole connected native engine subsystems and demonstrable integration over counts of isolated recovered functions.
