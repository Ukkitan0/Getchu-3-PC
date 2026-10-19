# Getchu-3-PC — bulk native compilation lane (experimental)

This lane replaces per-function manual C boot slices with whole-program static
recompilation using the **MIT-licensed, external** `snuri00/ps2-web-recomp`
Emotion Engine translator. No third-party code is copied into Getchu-3-PC.
The original native C diagnostic remains intact for differential tests.

Upstream: https://github.com/snuri00/ps2-web-recomp
Pinned reviewed source revision: `6526f4803d98e10de50bea3af104a4d26e9df033`.
At this revision, `recomp/main.cpp` confirms `eerecomp <elf> --entries <file> --out <dir>`
and reads whitespace-delimited hexadecimal guest entry addresses.
`CMakeLists.txt` confirms target `eerecomp`, a generated-game library and native
`ps2run` target. The Windows native build is **attempted but not validated**.

## One-pass entry discovery

The Python generator scans the full **1,351,830-word** retail executable text
inventory, 143 real static constructor addresses, and all recovered direct-JAL
**candidate** targets. It checks alignment and original-text bounds, de-duplicates
entries, and writes `entries.txt` suitable for the upstream recompiler.
By default only all **143 indirect constructor targets plus verified main-entry anchors**
are added: upstream traverses normal direct JAL calls automatically. The
`--seed-mode common` and `--seed-mode all` options opt in to larger speculative
seeds (2+ call sites, or all 12,228 raw target candidates respectively). It
is intentionally explicit that a JAL-shaped opcode can reside in unreachable
code or data; those targets are not certified function starts.

Run without any game bytes to get the metadata and clean entry list:

```
python tools/bulk_recompile.py --out fastlane/generated
```

On Windows, to build the entire *owned* US `SCUS_975.01` executable into
native C++, compile the upstream toolchain and attempt an actual x64 runner:

```
RUN_BULK_NATIVE_WINDOWS.bat -Elf "C:\path\SCUS_975.01"
```

The executable is SHA-256-checked against the exact US build. To attempt
opening the native runner after compilation:

```
RUN_BULK_NATIVE_WINDOWS.bat -Elf "C:\path\SCUS_975.01" -Disc "C:\path\Ape Escape 3 (USA).iso" -Run
```

The script requires Python 3, Git, CMake, Visual Studio 2022 C++ tools, and
possibly SDL2/wgpu-native dependencies; CMake may reject the current Windows
upstream because its native path primarily targets Linux/macOS. It prints/logs
that failure rather than producing a fake playable executable. It uses a
separate workspace, never overwrites the earlier native port, and never
publishes copyrighted executable or generated C++ code.

**Pass criteria for a playable build remain**: real code reaches the menu,
proper 3D geometry renders, controller works, game loop runs and an actual level
is entered. Generating thousands of entry units alone does not meet them.

To broaden optional entry seeds on Windows use `-SeedMode common` or
`-SeedMode all`. This can force data to be treated as code; it is not the
default. First measure the constructor-only recompilation before increasing.