# Windows native runtime portability + iterative whole-game recompilation

**Checkpoint date:** 2026-10-19. **Status: source and offline test checkpoint, NOT a tested native Windows game.**

## Why this changes the trajectory

We stopped implementing one MIPS constructor per turn and used existing MIT-licensed
`snuri00/ps2-web-recomp` as the game-independent C++ AOT and PS2 runtime.
The upstream source revision is pinned to `6526f4803d98e10de50bea3af104a4d26e9df033`.
The current public game repo includes only adapter source and fixtures, not an ISO,
retail ELF, translated proprietary game code, BIOS, or the upstream third-party sources.

## Implemented in this checkpoint

1. **Win32 runtime overlay:** idempotent, anchor-checked 11-source-edit overlay for
   SDL2 HWND recognition; WebGPU `WGPUSurfaceSourceWindowsHWND` creation; `WindowTarget`
   enum consistency; native Win32 process CPU/RAM measurements; platform-specific
   wgpu-native CMake linkage and a mandatory SDL2 window requirement. The full overlay
   validates all anchors before touching any file. It does not overwrite the owner's
   previous native-port attempt.
2. **Real Windows dependency pin:** launcher can download the official `gfx-rs/wgpu-native`
   `v29.0.1.1` Windows x64 MSVC archive, verifying SHA-256
   `7e67d7445c42aeb85e30f88930fd8d7d83ee769e3390aeb1ada75ebf3cf78132`
   from the official GitHub release metadata before extraction. Existing files are reused.
   `-SoftwareRenderer` is an explicit fallback; the GPU source patch itself has not
   been cross-compiled against the release headers in this environment.
3. **ClangCL native toolchain:** builds `eerecomp` and `vurecomp` through Visual Studio
   2022 ClangCL. Requires SDL2 via CMake/vcpkg; no silent headless-only success.
4. **Batch recompilation + iterative exploration:** retains the prior 143-constructor
   safe initial seed set. Optionally runs the compiled native game for N bounded headless
   exploration rounds and gathers missing guest indirect code addresses. Adds new
   validated, de-duplicated entry points atomically; recompiles all affected native
   game C++ units and repeats, without resetting previous discoveries. Also records
   VU1 microprograms through the upstream `PS2_VU_RECORD` path and translates them
   using `vurecomp` when captured.
5. **Windows GitHub Actions build smoke:** new workflow checks out pinned upstream,
   applies portability source edits, installs SDL2, builds the real upstream EE/VU
   recompiler, and attempts to link a native software-rendered `ps2run.exe` using
   an empty **test-only** registration stub, not any game code. The workflow is a
   test definition; its successful execution has NOT been confirmed in this session.
6. **Offline regressions:** 22 Python tests for seed generation, atomic Win32 patching,
   idempotence, counterexample rejection, original address validation, discovery merging,
   and launcher contract. Executed successfully on Linux, not in a Windows toolchain.

## How to run an actual Windows attempt

Preconditions: Windows 10/11 x64, Visual Studio 2022 with LLVM/ClangCL component,
Python 3, CMake, Git, Microsoft SDL2 development package available through vcpkg,
network access for clone/dependency downloads, and **your own** US `SCUS_975.01`
executable plus disc image. The pinned ELF SHA-256 is
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

```powershell
# Configure VCPKG_ROOT with the installed vcpkg path, and install SDL2:
& "$env:VCPKG_ROOT\vcpkg.exe" install sdl2:x64-windows

# Build and run native Windows executable, with 8 code-discovery loops and VU1 recording:
.\RUN_BULK_NATIVE_WINDOWS.bat -Elf "C:\path\SCUS_975.01" -Disc "C:\path\Ape Escape 3 (USA).iso" -ExploreRounds 8 -ExploreFrames 1800 -Run

# If wgpu-native cannot initialize on the local GPU, request software GS explicitly:
.\RUN_BULK_NATIVE_WINDOWS.bat -Elf "C:\path\SCUS_975.01" -Disc "C:\path\Ape Escape 3 (USA).iso" -SoftwareRenderer -Run
```

The original ELF and ISO stay under the user's local directories. Diagnostics and
translated outputs stay in `fastlane/private` by default, not GitHub.

## Verified vs not yet verified

**Verified:** Python unit/regression suite 22/22; patch fixtures reject unknown source,
apply twice without changes, and reject partial patching; original game inventory
and recovered constructor table remain intact; packaged source archive checksums.
The overlay anchors were inspected against the currently pinned upstream files.

**NOT verified:** successful GitHub Actions workflow run, Windows ClangCL compile,
Windows WebGPU surface creation, actual retail-ELF recompilation and runtime boot,
VU1 shader fidelity, console-to-Windows save conversion, scene geometry, title screen,
gamepad/keyboard input in the new runner, or gameplay. A Windows launcher script is
not a playable test build. Do not report constructor-entry coverage as proof that
all original functions are decompiled.

## Useful source material checked online

- Upstream source: https://github.com/snuri00/ps2-web-recomp
- Official WGPU v29 release: https://github.com/gfx-rs/wgpu-native/releases/tag/v29.0.1.1
- WebGPU Windows HWND C descriptor: https://webgpu-native.github.io/webgpu-headers/webgpu_8h_source.html
- Windows portability smoke: `.github/workflows/windows-fastlane-smoke.yml`

## Next engineering priority

Run the GitHub Windows build smoke. Resolve its concrete compile/link errors in
shared upstream runtime files, then run the private full-game AOT loop with the user's
own retail ELF/ISO. Only after a compiled native game reaches main menu and renders
real game geometry can a playable prototype be evaluated.