# Ape Escape 3 native Windows — disc boot, module and code-window milestone

Checkpoint date: 2026-10-19. This is a native executable **build route**,
not a verified playable Ape Escape 3 game.

## What changed

Compared with the previous AOT/143-callback smoke milestone, this checkpoint
ports the meaningful prerequisites from the pinned upstream PS2 game-port
pipeline into the owner-local native Windows script.

1. **One ISO can drive the build.** `tools/prepare_disc.py` calls the pinned
   upstream's tested ISO9660 reader to locate `BOOT2` in `SYSTEM.CNF`,
   extract the selected boot ELF, check the exact US retail executable
   SHA-256 and optionally confirm that a separately supplied ELF matches.
   Handles multi-directory BOOT paths and rejects traversal/mismatched files.
   No ISO or commercial game bytes are committed.
2. **Game modules and rewritten executable code.** Disc preparation extracts
   optional SNR2 modules if the boot executable uses them, detects executable
   zero-file-size code windows, creates a private overlay directory and feeds
   both through `eerecomp --modules/--overlays`.
3. **Actual code-window discovery and rebuild.** A bounded headless native run
   now sets `PS2_DUMP`, `PS2_VU_RECORD`, saves frames, records missing EE
   addresses, and creates a fresh memory card for each exploration round.
   Upstream `scripts/windows.py capture` turns the guest RAM window into
   named binary code images and translates raw window misses into
   `CODE_IMAGE:OFFSET` addresses. The same batch recompiler is re-run with
   all accumulated addresses and window/module images.
4. **Failure forensics.** Each exploration round has bounded process lifetime,
   persistent stdout/stderr, monotonic code discovery, code image counts,
   and JSON summaries. `tools/boot_report.py` aggregates metadata-only
   evidence for missing EE syscalls, IOP module imports, code misses, GPU
   initialization issues, VU1 microcode capture and available frame images.
5. **One-click test entrypoint.** `TEST_APE_ESCAPE_3_WINDOWS.bat` opens an
   ISO file picker when needed and runs the private Windows build, exploration
   and (experimental) native window launch. The script supports
   `-SoftwareRenderer`, `-ExploreRounds 0`, and `-RunOnly` to launch an
   already built native runtime with a persistent local PS2 memory card.
6. **Regression scope.** New synthetic ISO fixture is checked against the
   *actual pinned upstream* `scripts/disc.py` and `scripts/windows.py`
   in GitHub Actions. New independent game-free tests generate an ELF that
   executes freshly written code in an executable RAM window; the CI workflow
   captures its missing entry, rebuilds the captured image and asserts it
   returned value 143 via real Win32 ClangCL-native execution.

## Verified and pending

- Green GitHub Windows CI [39321437767](https://github.com/Ukkitan0/Getchu-3-PC/actions/runs/39321437767)
  validates the updated native pipeline's syntax, source-only AOT execution,
  Win32 GPU build, ISO9660 boot reader tests and executable-window discovery.
- The separate **synthetic executable RAM unpack/recompilation test** was added
  subsequently; check the newest GitHub Actions run before claiming it passes.
- The actual owner-supplied US game ELF+ISO is not available inside GitHub CI,
  and there is **no verified playable Windows Ape Escape 3 test build**.
- Native generated commercial code and game assets stay under the
  gitignored `fastlane/private`; do not upload that folder.

## One-click user-owned game attempt

On Windows with Visual Studio 2022 ClangCL, Git, CMake and Python 3 installed,
double-click:

```
TEST_APE_ESCAPE_3_WINDOWS.bat
```

Select your own US Ape Escape 3 ISO. The launcher bootstraps pinned upstream,
sets up SDL2 through vcpkg and WebGPU, prepares the disc and attempts the
full bulk native recompilation/exploration/launch in one flow.

To run an already-built executable from the same selected ISO without rebuilding:

```powershell
.\TEST_APE_ESCAPE_3_WINDOWS.bat -RunOnly
```

To bypass the file picker and provide an ISO explicitly:

```powershell
.\TEST_APE_ESCAPE_3_WINDOWS.bat -Disc "D:\Games\Ape Escape 3 (USA).iso"
```

The key evidence files after any attempt are:

```
fastlane/private/build.log
fastlane/private/owned_disc/disc_prepare.json
fastlane/private/explore_summary.json
fastlane/private/boot_report.json
fastlane/private/explore_round_01.stderr.txt
fastlane/private/shots/
```

Do not infer a functioning title screen, graphics, controls or gameplay
from compiler CI or game-free callback tests. Those require the real disc
being run on the user's computer and observed frames.
