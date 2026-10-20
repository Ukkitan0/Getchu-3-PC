# Getchu-3-PC — Original OSD read/modify/restore checkpoint

October 19, 2026. **Native C experimental EE boot diagnostic, NOT PLAYABLE.**

## Recovered original retail execution

Previous boundary: `GetOsdConfigParam` (`0x4B`) syscall at guest PC
`0x00435B94` after 323,510 counted original instructions in composed
experimental boot segments. With the original retail PS2 EE instruction bytes
present in a mapped guest memory bus, the new bounded C continuation executes
**44 additional original instructions**, ending at parent PC **`0x0043FE98`**
(323,554 counted original instructions on the composed test path).

The nested game initializer at `0x0043FF18` makes the following original calls:

1. `GetOsdConfigParam(0x4B)` to `sp+0`.
2. Clear bits 13–15 of the returned 32-bit word, set bit 13, and call
   `SetOsdConfigParam(0x4A)` from `0x0043FF48` with `sp+4`.
3. `GetOsdConfigParam(0x4B)` again to `sp+4` and test the resulting setting.
4. `SetOsdConfigParam(0x4A)` again from `0x0043FF58` with the saved original
   value at `sp+0`, restoring the original configuration.
5. Return through the genuine EE stack-frame restores, a branch/delay slot,
   then into the parent's next call site at `0x0043FE98`.

All guest `JAL`, `JR`, delay slots, `LW`, `LD`, `SW`, bitwise arithmetic,
register restores, and branch logic above use original opcodes verified
against the user's PCSX2 EE RAM reference. The *two syscall services* are a
narrow, stateful C compatibility model, **not actual Sony BIOS execution**.
The main continuation only supports the original verified branch in which
the modified config persists on re-read and is subsequently restored. A path
where the attempted OSD write does not persist requires separate handling.

The original sequence is verified against PS2SDK `ConfigParam`, with one packed
32-bit word and syscall numbers `0x4A` (setter), `0x4B` (getter):
https://github.com/ps2dev/ps2sdk/blob/master/ee/kernel/include/osd_config.h

## Explicit configuration and safe behavior

There is **no** assumed BIOS language, timezone, or console option value.
`ae3_ee_osd_bind()` requires one explicit packed configuration word;
without binding the getter/setter fail atomically. The guest reads and writes
its own mapped EE RAM, and the stateful compatibility configuration is updated
only after a valid, aligned original syscall and accessible guest data.

The verified standalone fixture starts with `0x00004019` (arbitrary TEST input),
observes the game's temporary value `0x00002019`, then confirms the final
value is **restored to `0x00004019`**. Repeated getter/setter calls, branch
delay slots and all saved registers are checked against a single-step/batched
execution copy. Injected invalid syscall opcodes, invalid mappings and
unbound configuration never get silently promoted to success.

## Tested locally

- GCC optimized native C test: PASS.
- Clang AddressSanitizer + UndefinedBehaviorSanitizer: PASS.
- Clang-CL `/W4 /WX` Windows x64 COFF object: PASS (not an EXE).
- Full Linux boot diagnostic: compiled and linked successfully, NOT run
  against retail `SCUS_975.01` because only the earlier PCSX2 savestate is
  currently accessible.
- Previously existing thread/OSD entry and thread-creation tests: PASS.
- **290/290** pinned retail MIPS opcode entries across eight source tables
  equal the user's post-boot 32 MiB EE RAM reference; this is *code* evidence,
  not genuine startup BIOS/device state.

Standalone test:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_memory_bus.c \
  ae3_boot_osd_roundtrip.c test_boot_osd_roundtrip.c -o build/test_boot_osd_roundtrip
./build/test_boot_osd_roundtrip
```

Integrated opt-in boot diagnostic (requires your original ELF):

```sh
./build/ae3_boot_probe SCUS_975.01 --test-ee-boot-model \
  --test-sema-hle --test-ee-alias --test-osd-roundtrip=00004019
```

Windows: compile via `BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat` in a VS Developer
Command Prompt, set `AE3_BOOT_TEST_OSD_CONFIG=00004019` and `AE3_ELF` explicitly.
This test flag deliberately opts in to using the supplied OSD word.

## Next original execution target

The parent JAL at `0x0043FE98` calls `0x0043F470`, which contains further
startup/syscall initialization. This call has **not** been executed by the
new module. Authentic EE BIOS thread scheduling, IOP communication, GS/VU
rendering, audio and native gameplay remain unimplemented.