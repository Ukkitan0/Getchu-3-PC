# Recovery of private opcode inventory from original user-owned ELF

Date: 2026-10-19. **Experimental, NOT playable.**

## Corrected public-repository blocker

The original repository ZIP includes a user-recovered opcode inventory, but the
canonical public GitHub repo intentionally does not publish `analysis/full_elf`
original machine-code bytes. The initial Windows GitHub Actions smoke job failed
when it attempted to run a full-game inventory-based test with those unavailable
files. Its first 14 standalone Windows-safe tests passed. The smoke workflow was
updated to run the independent portable test suite only; it is not a game build.

`tools/derive_elf_inventory.py` now parses a validated **32-bit little-endian
MIPS ET_EXEC ELF**, checks every executable `PT_LOAD` code segment, and reconstructs
original instruction words and a direct-JAL target histogram into a **private**
working directory. It rejects misaligned or out-of-bounds segments, wrong ELF
machine, bad entry points and oversized executable ranges. The bulk launcher
verifies the user's original executable SHA-256 BEFORE calling this generator.

The existing user-supplied archived opcode inventory was used to independently
reconstruct every recovered raw direct-JAL target. The rebuilt result exactly
matches all **12,228 candidate addresses** and their per-target call-site counts
in `analysis/full_elf/direct_jal_targets.tsv`. Those candidate addresses are
not proven function starts or evidence of successful whole-game translation.

The updated `tools/bulk_recompile.py` uses existing private inventory when
present (no regression for old complete checkpoint ZIPs). If these files are
absent, it derives them from `--elf` under `<out>/derived_from_owned_elf/`, then
continues into candidate selection and the upstream full-game translation.
The private files should never be uploaded back to the public repo.

## Offline tests

- 28/28 Python unit tests pass locally: 20 Windows engine/ELF reconstruction
  suite cases plus 8 existing full-inventory tests.
- Artificial 32-bit MIPS ET_EXEC fixture tests generation, JAL counting,
  missing-inventory fallback, invalid machine/endian/segment rejection,
  and missing-ELF guidance without any copyrighted source.
- Real recovered opcode inventory cross-check: all 12,228 target counts match.
- The Windows GitHub Actions test workflow must still successfully run to
  establish upstream compiler/toolchain compatibility.

## Validated vs outstanding

**Verified:** Linux offline tests; private metadata regeneration on synthetic
ELF; equivalence of recovered 12,228 original direct-JAL target records.

**Unverified:** Windows native compiler, WebGPU window creation, actual
recompilation from owner ELF on Windows, runtime with ISO, original main menu,
3D gameplay and controls. None of this is a playable build.

## Real Windows source-only build achievement

A source-only GitHub Actions job compiled the x64 ClangCL native EE/VU
recompilers and linked a software-GS ps2run.exe using empty hooks. This is a
Windows native-runtime compilation milestone, not original-game execution.
The corrected CI workflow will also attempt to compile the Windows WebGPU path.