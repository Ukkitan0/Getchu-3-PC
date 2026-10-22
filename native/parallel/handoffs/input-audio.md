# Input, Gadgets & Original Audio Runtime — specialist handoff

**Specialist branch:** `parallel/input-audio`
**Integration target:** `native/original-elf-engine-recovery-20261020`
**Workstream state:** Source-backed Lua sound/gadget command boundary and physical host pad transport implemented and locally tested; original player pad decision tree, complete gadget execution, original audio mixing/decoding and playable Windows integration are **not** recovered.

## Commits (this branch ONLY)

- `de3cf850826cf73965943444d0c814457fa7c57b` — six new isolated native files: `original_pad_runtime.hpp/.cpp`, `original_pad_win32.cpp`, `original_audio_runtime.hpp/.cpp`, `original_input_audio_runtime_test.cpp`.
- `3f27ce8bf35fa5147fbc2cb5fe402c7e5ceede93` — original-source `se` parameter fields, **source-correct retail Lua `wait` default of 1.0s**, tests, and an offline SHA-pinned retail ELF audit.
- This document is the category-owned handoff. All other category branches, `native/CMakeLists.txt`, `native/src/app_win32.cpp`, workflow config, and `main` were left unchanged.

## Direct retail USA ELF evidence

Recovered from the owner-provided original USA ISO, reassembled privately from seven previously supplied parts. **Verified private ISO SHA-256** `5b4c6272c9daf190b3f72081f124edd7ea2af3cc0dafeaef206ded97a022fd09`, size 2,174,091,264. Extracted original `SCUS_975.01` **SHA-256** `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`, size 6,488,148. Neither ISO, ELF nor retail audio data was committed.

| Original USA virtual address | Verified meaning / dependency |
| --- | --- |
| `0x512000` | `set_player_weapon` Lua binding compares the original literals `empty`/`net`/`club` at `0x6ECC10`/`0x6ECC18`/`0x6ECC20`, uses selectors 0/1/2 and fallback 3, then JAL `0x266CF4` at `0x512034`. **This is script-directed selection, not proof of gadget-button mappings.** |
| `0x2191D4` | Source Lua `se` command parser, fields `name` `0x6E0AB8`, `target_name` `0x6E2D90`, `vol` `0x6E2DF0`; original **1.0f** default literal `0x6E2DF4` loaded at `0x219224`; JAL `0x373264` at `0x219264`. The constructor allocates original 0xA0-byte command and writes float to +0x90. Neither the host layout nor audio playback is reconstructed merely from this. |
| `0x2190D8` | Lua `se_stop` dispatch; JAL original event constructor `0x3732FC` at `0x219138` (0x70-byte command). |
| `0x219454` | Lua `add_entity_se` parser: original `exdb_se_id`, `entity_name`, `watch_rollerhandle_name`, `sign`, `is_2d`, `is_3d` keys; JAL `0x3738DC` at `0x2195A0`. Detailed semantics and backend remain to be recovered; raw Lua parameter table is preserved in our event. |
| `0x2195D0` | Original `kick_entity_se` c_ctrl dispatch (verified source dispatch table); event payload and callee still need deeper recovery. |
| `0x21937C` | Lua `wait` command: key `time` at `0x6E2DF8`, default **1.0f** literal at `0x6E2E00` loaded at `0x21939C`, JAL `0x37321C` at `0x2193CC`. The inherited timeline's erroneous default 0 was corrected to 1.0 without altering explicit times. |
| `0x1BB9A8` | Original five-field audio resource descriptors and `volume_scale` remain **Assets-owned** (`original_audio_descriptor.hpp`), passed through unchanged as optional original resource values. No invented audio asset parser was introduced. |

`native/tools/verify_original_input_audio_retail.py <private-SCUS_975.01>` validates the original USA ELF hash, original JAL instructions to constructors, 12 original string literals, and original 1.0f loads. Verified it passes against the extracted source. No game binary bytes are emitted.

**Important controller provenance limit:** Initial host physical DualShock-style button indexes and XInput-to-pad transport are a platform adapter only. The original PS2 retail pad reader, its game-state button masking/dead-zone/pressure curves, menu/gameplay transition handling, the actual per-frame input consumer and player gadget-action call chain are **not yet verified**. `PadRecorder` retail string at `0x6ED0A0` and button label strings do **not** establish action bindings. The adapter intentionally has **no default invented keyboard control map** and never injects movement commands.

## Actual delivered APIs

- `ae3::OriginalPadObservation` preserves connection, positive pressed-mask transport, full LX/LY/RX/RY byte samples, and L2/R2 host trigger observations. `OriginalPadFrameRuntime::advance(gameFrame, observation)` yields **newlyPressed/newlyReleased** edges once per externally supplied increasing native frame; handles pad loss/reconnection/reset without fake repeat. The host frame scheduler determines timing; these edge rules are adapter behavior, not a claim about original PS2 frame filtering.
- `ae3::pollWin32OriginalGamepad(void* hwnd, unsigned index)`: XInput dynamic-library loading; physical XInput face, shoulder, d-pad, sticks and triggers. `ae3::pollWin32OriginalKeyboard(void* hwnd, const OriginalKeyboardBindings&)`: explicitly supplied Windows VK mappings only (defaults unbound); focus restricted. Neither maps physical controls onto fabricated gameplay actions. No Windows device run was available here.
- `ae3::originalSetPlayerWeaponSelection_512000(name)` faithfully returns original 0/1/2/3 choices; `forwardOriginalGadgetSelection(optional selection, frame, sink)` delivers only an **explicit game-script** selection and never synthesizes an activation. Native Lua script binding already stores a selection separately; Gameplay/Runtime must select source of truth.
- `ae3::originalSoundRoute(opcode)` accepts only verified original sound commands: `se`, `se_stop`, `add_entity_se`, `kick_entity_se`. `ae3::OriginalSoundEventRuntime::enqueue(original NativeControllerEvent, entityName, optional OriginalAudioDescriptorValues)` retains **the entire original event/payload/time** and optional Assets-provided descriptor. `pump(OriginalSoundEventSink&, limit)` delivers in order, stops on backend rejection and does not emit fake samples; `clearOnOriginalStageTeardown()` clears a departing stage's commands. Original backend/playback is **not implemented**.
- `ae3::originalSeLuaFields_2191d4(event)` extracts original recognized `name`, `target_name`, `vol`; an absent `vol` uses verified source 1.0f. Non-numeric coercion via `0x3E5BD0` is still unresolved and all raw script parameters remain available.

## Local tests — what actually ran

Host: Debian Linux, `g++ (Debian 14.2.0-19) 14.2.0`, Python 3.13.5. Native C++ boundary tests compiled with **local API-equivalent trimmed header mirrors** of existing shared headers (not the entire real CMake dependency tree); the tested `original_pad_runtime.cpp`, `original_pad_win32.cpp`, `original_audio_runtime.cpp`, `original_input_audio_runtime_test.cpp` were the submitted implementation sources. Ran **and passed** both:

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -I native/include \
  native/src/original_pad_runtime.cpp native/src/original_pad_win32.cpp \
  native/src/original_audio_runtime.cpp native/tests/original_input_audio_runtime_test.cpp \
  -o /tmp/ae3_original_input_audio_test && /tmp/ae3_original_input_audio_test

g++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -I native/include \
  native/src/original_pad_runtime.cpp native/src/original_pad_win32.cpp \
  native/src/original_audio_runtime.cpp native/tests/original_input_audio_runtime_test.cpp \
  -o /tmp/ae3_original_input_audio_asan && /tmp/ae3_original_input_audio_asan

python native/tools/verify_original_input_audio_retail.py /private/SCUS_975.01
```

Result: `ORIGINAL_INPUT_AUDIO_BOUNDARY_TEST_PASSED` in optimized build and ASan+UBSan; `ORIGINAL_INPUT_AUDIO_RETAIL_EVIDENCE_PASSED` from authenticated USA ELF. Tests cover frame transitions/unplug/replug/reset, script gadget mapping, source sound route VAs and order, queue backpressure, raw Lua field preservation, volume-default interpretation, and optional asset descriptor pass-through.

The **additional `controller_timeline_test.cpp` retail default-wait test is committed but not locally run**: it needs the full original Lua 5.0 CMake build. The **Win32/XInput code path, real audio device/decoder, full CMake/CTest, game boot, input-to-original-actor integration and audible in-game source sound have not been compiled/run/verified**. No GitHub Actions were launched.

## Exact integration request — Windows Runtime / Assets / Gameplay

Integrator only: add to `Getchu3Assets` in `native/CMakeLists.txt`:

```cmake
target_sources(Getchu3Assets PRIVATE
  src/original_pad_runtime.cpp
  src/original_pad_win32.cpp
  src/original_audio_runtime.cpp)
add_executable(Getchu3OriginalInputAudioTests
  tests/original_input_audio_runtime_test.cpp)
target_link_libraries(Getchu3OriginalInputAudioTests PRIVATE Getchu3Assets)
add_test(NAME OriginalInputAudioSourceBoundaries
  COMMAND Getchu3OriginalInputAudioTests)
```

The Win32 adapter loads XInput dynamically and does not require a new import library; `user32` is already linked to the app. Keep original `Getchu3ControllerTimelineTests` to verify corrected wait semantics.

Windows Runtime should sample native gamepad once per native game frame, call `OriginalPadFrameRuntime::advance()`, and **do not feed button edges into the experimental `NativeWalkingController`** as real Ape Escape 3 movement. Gameplay needs to finish the original pad-reader-to-player/gadget call chain to consume the physical state. Use `NativeStageWorld::playerWeaponSelection()` only when the real script executes, not to fabricate an input-driven gadget state.

Original stage controller events should be enqueued to `OriginalSoundEventRuntime` by the integration owner **in addition to** all unrelated events continuing to Gameplay/Stage. Assets team supplies decoded source resource descriptors; playback sink must return false until original asset formats and sound event implementation are verified, rather than playing fake tones or discarding commands. Full engine must restore proper stage teardown and backpressure handling.

## Remaining critical original-code recovery

1. Identify exact original retail pad acquisition, sample layout, event/hold/edge timing and per-game-state processing via authenticated ELF; recover original gameplay gadget selection/activation rather than binding buttons by guesswork.
2. Trace `0x373264`, `0x3732FC`, `0x3738DC` and `kick_entity_se` to actual source sound manager, resource lookup and renderer; distinguish `se` from BGM and reverb.
3. Work with Assets to connect **source-backed** `*.mid`/`*.hd`/`*.bd` descriptors to decoded original audio resource objects; separately recover original sequencing, gain, speaker output and track transitions. No original sound assets are published.
4. After original gameplay callback chain is recovered, integrate native button samples with authentic player action transitions; conduct full Windows device tests (multiple controllers, disconnect/reconnect, menu focus, native frame timing) and real-game audible verification.

**Completion status:** source-backed Lua command boundaries and host transport exist, private ELF audit and isolated C++ tests pass, **not integrated/playable**. No PS2 emulation, fake gameplay/sound, copyrighted game bytes, cross-owner edits or manually dispatched workflows.


---

## Pass 2 — original retail pad reader and analog control (2026-10-21)

**Commit:** 9e40e4a67575018e610456f5f8f2d9921393fd0c, branch parallel/input-audio only.

### Original ELF source recovery

Source: private original USA SCUS_975.01, SHA-256 5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63. Verified directly using original executable. No source binaries were committed.

- **0x26EAA0..0x26EAE8:** game-owned input manager initialization. Calls pad library setup 0x4054F8, allocates a 0x50-byte manager and initializes via 0x517C1C. Native adapter does NOT emulate this pad library.
- **0x5179B0..0x517A90:** reads 16 original pad channels through function 0x4059E8. Immutable retail channel table at 0x6ED0C8 is [0,3,4,6,7,5,12,14,15,13,10,8,1,11,9,2]. Fetches four analog bytes with channel codes [0x12,0x13,0x10,0x11], ordered RX, RY, LX, LY. These are source channel IDs, NOT presumed host XInput bit positions.
- **0x51785C..0x5179B0:** source device read and state transition: newly pressed = current & ~previous. On unavailable controller, current digital mask resets to 0 and all four stick bytes to 0x80. Released-mask value in the native adapter is host convenience, not an established original field.
- **0x517788..0x51785C:** source analog axis normalization: (byte-128) * float32(0x3C088888) (~1/120). Below squared magnitude 0.140625 (radial deadzone 0.375), outputs zero. Otherwise r=sqrt(x*x+y*y), multiply by (r-0.375)/(r*0.625), then clamp each component to [-1,+1]. Original constants at 0x6ED0AC/0x6ED0B0/0x6ED0B4/0x6ED0B8/0x6ED0BC/0x6ED0C0. Recovered original MIPS floating operations and branch, not guessed host stick settings.
- **0x26EB34..0x26ED54:** game tick polls each of two controller ports with 0x51785C, computes axes with 0x517788, exports two 0x18-byte game input records at original memory 0x6496F8 (port 0), 0x649710 (port 1). Record slots: +0 u32 held bits, +4 u32 new-press bits, +8/+12 float right X/Y, +16/+20 float left X/Y.

### Files and actual native APIs committed

- native/include/ae3/original_retail_pad.hpp
- native/src/original_retail_pad.cpp
- native/tests/original_retail_pad_test.cpp
- native/tools/verify_original_retail_pad.py

Integration: instantiate ae3::OriginalRetailPadReader and call reader.sample(OriginalRetailPadChannelSource&, deviceConnected) once per native game input tick. Pass the resulting sample to ae3::originalRetailGamePadState_26eb34(sample) to obtain an original-layout **0x18-byte** game controller record. Implement OriginalRetailPadChannelSource::queryOriginalPadChannel_4059e8(channel) with a host device mapping matching the original *query channel*, rather than treating the original query code as an arbitrary XInput mask or inventing gadget controls. Hand the record into the authentic original game input consumer when recovered; do not route it through the temporary native walking prototype. Other teams own the app/gameplay integration.

**Integrator-only CMake recipe:** Append src/original_retail_pad.cpp to Getchu3Assets. Define a Getchu3OriginalRetailPadTests executable with tests/original_retail_pad_test.cpp, link Getchu3Assets, add CTest OriginalUSAControllerReaderAndAnalog. This subsystem did not edit CMake or app_win32.cpp.

### Actual tests

Locally compiled the **exact SHA-matching committed source/header/test blobs** with g++ 14.2.0 on Debian Linux:

- Optimized: g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -I native/include native/src/original_retail_pad.cpp native/tests/original_retail_pad_test.cpp -o /tmp/retail_pad && /tmp/retail_pad — PASSED.
- Sanitizers: g++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -I native/include native/src/original_retail_pad.cpp native/tests/original_retail_pad_test.cpp -o /tmp/retail_pad_asan && /tmp/retail_pad_asan — PASSED.
- Private executable: python native/tools/verify_original_retail_pad.py /private/SCUS_975.01 — ORIGINAL_RETAIL_PAD_ELF_AUDIT_PASSED. Checks 4 exact original function-body hashes, 16 source channel entries, 6 float constants, 9 JAL callsites, and controller edge/clear instruction words.

Tests cover neutral and dead-zone values, nontrivial radial scaling, component clamping, all 20 query-channel calls in original order, held/pressed/released transitions, disconnect/reconnect, and original-size game state records. All test invocations passed.

### Remaining blockers (do not claim working Windows input yet)

Still must resolve original per-game-state button interpretation and gadget action call tree, controller channel mapping from physical Windows input, hardware controller device connection semantics, direct original game-state consumer, genuine SoundSys mixing/streaming/resource lookup and original music system. The original Win32 app was not linked or booted and live input/audio were not tested. No GitHub Actions triggered and no game data published.


---

## Pass 2b — connected retail sound-command constructors and entity sound fields

**Commit:** 17ff9527ccda5a721628359018a13e9b8b1cca56 on parallel/input-audio.

Direct ELF calls and size arguments verified from original private SCUS_975.01:

| Original c_ctrl command | Lua parser entry | Source event constructor call | Original allocation |
| --- | --- | --- | --- |
| se | 0x2191D4 | 0x219264 -> 0x373264 | 0xA0 (160) bytes |
| se_stop | 0x2190D8 | 0x219138 -> 0x3732FC | 0x70 (112) bytes |
| add_entity_se | 0x219454 | 0x2195A0 -> 0x3738DC | 0xD0 (208) bytes |
| kick_entity_se | 0x2195D0 | 0x2196C0 -> 0x373960 | 0xB0 (176) bytes |

The constructor addresses, allocated bytes and command parser entrypoints are SOURCE provenance only, not host allocation commands, original playback entrypoints or a claim that audio samples are decoded.

**Actual interface additions:** OriginalSoundRoute now exposes originalConstructorVa and originalObjectBytes for the real command object type. originalEntitySoundLuaFields(event) recognizes exactly source-backable keys: exdb_se_id and entity_name for both entity sound commands; watch_rollerhandle_name, sign, is_2d and is_3d for add_entity_se; mark_name for kick_entity_se. Absence remains an optional value, and the complete original parameters remain in the audio event untouched. This typed extractor does not claim to reproduce the incomplete original Lua conversion/default system.

**Verification:** Native C++20 optimized and ASan/UBSan boundary tests passed locally for these functional additions using the local source variants; original ELF audit with added constructor JAL, event-size and literal-key checks also passed locally. The newly committed remote source changes were not independently recompiled after remote upload; the remote input/audio test Git blob did match the exact locally tested test file. No Windows sound device, original bank decoder or full native game boot was tested.

**Integrator:** No new CMake sources required beyond original_audio_runtime.cpp and the existing original_input_audio_runtime_test.cpp in the previous handoff. Recompile/CTest on Windows after selecting this commit. The event backend must use original audio assets/resource IDs supplied by Assets and recovered original sound manager; a named event alone is not permission to synthesize sounds.

**Remaining sound work:** Trace vtable-driven SE execution from 0x373264 to original sound manager, original name/EXDB resolution, BGM MIDI/HD/BD sequencing, stream and reverb control, native device output and state transitions. Do not infer played sound from constructor invocation. No copyrighted audio was committed.


---

## Pass 2c — physical host state through the verified original channel table

**Commit:** 1e97fbbca81b640e9e8ade4c94278d5c6acfa917, branch parallel/input-audio.

A new OriginalHostPadChannels adapter implements OriginalRetailPadChannelSource directly from the already-owned OriginalPadObservation produced by pollWin32OriginalKeyboard or pollWin32OriginalGamepad. Original 0x4059E8 channel codes 0..15 select a digital pad button in the standard PlayStation bit order; channels 0x10/0x11/0x12/0x13 supply raw LX/LY/RX/RY byte observations. Reader 0x5179B0 then queries them in the *original retail lookup order*, derives original newly-pressed bits and passes unmodified raw sticks through the independently recovered original radial transform 0x517788. Host receives no fabricated new gameplay actions or key bindings.

Integration call sequence: OriginalPadObservation physical = pollWin32OriginalGamepad(hwnd,index) (or pollWin32OriginalKeyboard with *explicit* user/integrator bindings); OriginalHostPadChannels channels(physical); OriginalRetailPadSample originalPad = originalReader.sample(channels,physical.connected); OriginalRetailGamePadState_26eb34 gameState = originalRetailGamePadState_26eb34(originalPad). The original gameplay team, not this team, must connect gameState to original input consumers and gadget actions.

**Focused tests:** physical original Cross/Start observations map to actual retail output bit positions 7/1, RX/RY/LX/LY reorder is correct, analog source normalization preserves right/left stick identity, and unsupported 0x14 channel is rejected. Source/header/test Git blob hashes verified identical to locally executed optimized C++20 and ASan/UBSan source. BOTH tests passed. Only portable non-Windows branch compiled; actual XInput/Windows hardware is still untested.

This is a source-backed channel interface and original polling transformation, NOT PS2 emulation or a fully integrated player gadget controller. Original menu/gameplay button consumers, active gadget actions, and actual game-loop integration still need recovery. No CMake or app entry changes and no GitHub Actions.
