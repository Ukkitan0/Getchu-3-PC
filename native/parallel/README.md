# Parallel Ape Escape 3 original-engine recovery

Eight independent subsystem conversations now work on separate GitHub branches of **Ukkitan0/Getchu-3-PC**, sharing an identical verified native baseline.

- **Integration baseline:** `native/original-elf-engine-recovery-20261020`
- **Verified original source at split:** `879c347c1bfca1013e2fb514f6a4ebb773883fc5`
- **GitHub default `main`:** intentionally NOT modified or merged into this native effort
- **Private original game source:** USA `SCUS_975.01`; never commit the executable, game scripts or copyrighted asset bytes.
- **Workflow:** the existing `.github/workflows/native-windows.yml` is `workflow_dispatch` only. Do not dispatch it automatically.

## Open one conversation for each category

| # | Category | Specialist branch | Brief and copy-ready kickoff | Individual handoff |
|---|---|---|---|---|
| 1 | Windows native runtime & integration | `parallel/windows-runtime` | [brief](workstreams/windows-runtime.md) | [status](handoffs/windows-runtime.md) |
| 2 | Original actors, gameplay and AI | `parallel/gameplay-engine` | [brief](workstreams/gameplay-engine.md) | [status](handoffs/gameplay-engine.md) |
| 3 | Navigation and pathfinding | `parallel/navigation` | [brief](workstreams/navigation.md) | [status](handoffs/navigation.md) |
| 4 | VU0 mathematics and collision | `parallel/vu0-physics` | [brief](workstreams/vu0-physics.md) | [status](handoffs/vu0-physics.md) |
| 5 | Original graphics, models and animation | `parallel/graphics-animation` | [brief](workstreams/graphics-animation.md) | [status](handoffs/graphics-animation.md) |
| 6 | Original resource and asset formats | `parallel/assets` | [brief](workstreams/assets.md) | [status](handoffs/assets.md) |
| 7 | Original Lua 5.0 scripts and bindings | `parallel/lua` | [brief](workstreams/lua.md) | [status](handoffs/lua.md) |
| 8 | Input, gadgets and sound | `parallel/input-audio` | [brief](workstreams/input-audio.md) | [status](handoffs/input-audio.md) |

## Getting started

1. Open your workstream's **brief** link and paste its **Kickoff prompt** into a new chat.
2. Give that chat access to its own GitHub branch (already created). Each chat commits code **only to that branch**.
3. Read that team's handoff for completed work, precise evidence, blockers and build integration requests.
4. Use this original native recovery conversation as integrator; it selectively incorporates specialist commits into the baseline and handles the shared native CMake/build system.

## Boundaries

The ownership matrix is in [WORKSTREAMS.md](WORKSTREAMS.md). Full integration and safe merging are in [INTEGRATION_PROTOCOL.md](INTEGRATION_PROTOCOL.md). Files changed in another subsystem must be requested through **your own handoff** rather than edited concurrently. Shared retail-ELF investigation is allowed, but original ELF/ISO/PCK/Lua must stay owner-private.

These are **separate conversations**, not autonomous background workers. GitHub is the shared durable record. Their branches do not automatically merge or synchronize.
