# Native Windows Runtime & Integration — specialist handoff

**Specialist branch:** `parallel/windows-runtime`
**Integration target:** `native/original-elf-engine-recovery-20261020`
**Status at branch creation:** Not started on the parallel branch. Inherited source remains unmodified.

The owner of this category updates **only this handoff document** on its own dedicated branch. The integrator reviews and merges from here; do not edit the shared integration documents.

## Completed source recovery and commits
- _No parallel-category commits yet._

## Original executable / game-resource evidence
- _List authentic source function VAs, asset filenames and SHA verification; do not commit private game bytes._

## Actual native tests run
- _List exact compiler/tool versions, command, passed/failed results. Distinguish local isolated tests, complete CMake/CTest and actual native Windows game boot._

## Integration request for Windows Runtime team
- _List any new .cpp/.hpp, CMake target changes, linking needs and precise expected native API signatures._
- _Describe required edits to other teams' files without editing them directly._

## Unfinished source behavior and blockers
- _List original engine subroutines not yet recovered and evidence missing. Avoid percentages unless measurement is sound._

## Next batch of connected native implementation
- _Identify a substantial remaining subsystem and how its original data/control flow will be tested._

## Rules
No PS2/EE/IOP emulation, no substitute gameplay, no copyrighted ELF/ISO/PCK/Lua checked in, no manually triggered GitHub Actions and no direct changes to the baseline or default main. Commit only source/tests/docs to `parallel/windows-runtime`.
