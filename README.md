# Getchu-3-PC — Ape Escape 3 native Windows decompilation

The **canonical decompilation/source-recovery project** for the PS2 NTSC-U *Ape Escape 3* (`SCUS_975.01`). Our goal is a complete **native Windows port**, not an emulator frontend or mockup. Historic recompilation builds are reference material only.

> **October 18, 2026 checkpoint:** The source translators compile tested native C for a **restricted subset** of the original MIPS R5900 executable. **This is not a playable Windows game**, and this repository does not contain the original game, assets, or fully bootable runtime.

## Measured source recovery

| Layer | Distinct candidate entries | Notes |
|---|---:|---|
| Scalar/forward/loop leaf translators | 1,470 | 1,163 straight-line + 168 forward-CFG + 139 bounded loops |
| Direct-JAL callers | 30 | Call a supported leaf |
| Nested-JAL callers | 1 | Stack depth >1 exercised |
| Whitelisted dynamic-JALR callers | 31 | Unknown targets explicitly rejected |
| **Forward branch and call wrappers** | **72** | New in this checkpoint: forward BEQ/BNE/BLEZ/BGTZ/REGIMM or J with JAL/JALR |
| **Total distinct entrypoints** | **1,604** | Candidate entries are not guaranteed full function boundaries |

The scalar resumption generator maps 9,390 original instruction locations (including return instructions). Prior to this checkpoint, call-capable wrappers represented 940 additional locations; the new branch/call pass represents another 1,432. Thus current translated **site total is 11,762** for these separately enumerated layers. Do **not** interpret it as 11,762 proven reachable or semantically identified instructions; aggregate reports use different instruction counting conventions.

### Key implementations

- `bulk_lift.py`, `cfg_lift.py`, `loop_lift.py`: conservative low-64-bit scalar MIPS R5900 leaf candidates.
- `resume_lift.py`: resumable guest-PC, branch and delay-slot execution.
- `call_lift.py`, `stack_lift.py`, `extend_stack.py`: direct JAL, nested calls, dynamically checked JALR, and **forward control-flow callers**.
- `test_*`: independent reference execution of the original retail MIPS instruction words, comparing GPRs, byte-exact RAM, instruction counts, PC, pending slot state, and errors.
- `ae3_native_dispatch.c/h`: common entrypoints for the above restricted routines; not a PS2 hardware runtime.

## Run the code from your own copy

Use your own unmodified USA executable, SHA-256:

`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`

It can be passed using the `AE3_ELF` environment variable or kept locally as `SCUS_975.01`. **Do not commit or redistribute the retail game or generated game-code C.**

Linux (Python 3.10+, C compiler):

```bash
AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh
```

Windows (Python 3 and clang-cl / Visual Studio C++ tools):

```bat
set AE3_ELF=C:\ApeEscape3\SCUS_975.01
BUILD_WINDOWS_OBJECTS.bat
```

The Windows script builds `.obj` files for further integration; **it does not make a playable EXE**.

## Verified October 18 test milestone

- `test_stack.py 2`: **268** original-MIPS vs native-C trials, **2,194** compared execution slices across **134** caller candidates.
- `test_branch_call_paths.py`: forced the taken and not-taken branches of retail code at `0x003817D0`, including the indirect callee `0x003B3CE0` on the latter path. Both completed and matched the oracle (registers, memory, state).
- Rejected/unsupported `JALR` targets no longer partially mutate the saved target state before returning a fault. Added an explicit regression assertion.
- Linux native dispatcher integration passed and `clang-cl` successfully compiled the updated stack module to a Windows x64 COFF object.
- Earlier scalar/CFG/loop/resume/direct-JAL suites passed individually; full end-to-end script rebuilding every output did not complete within this environment's command limit.

See [CHECKPOINT_BRANCH_CALLS.md](CHECKPOINT_BRANCH_CALLS.md) and [STATUS.md](STATUS.md) for limitations.

## Remaining work

The executable still requires broad function coverage, verified 128-bit GPR/MMI/COP1/VU support, EE/IOP scheduling and services, VIF/GIF/GS rendering, disc/asset handling, audio, controllers, save logic, and a real Windows game boot/integration test. Only supported scalar candidates can run today, and indirect calls must resolve to known entries. Nothing in this repository claims completed gameplay or graphics.