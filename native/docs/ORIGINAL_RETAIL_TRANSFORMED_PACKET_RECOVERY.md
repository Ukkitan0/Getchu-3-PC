# Graphics recovery: actual USA packet transforms and model stream boundaries

Source: original USA `SCUS_975.01`, verified SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. Game binaries and assets remain private.

## Recovered code

New original-engine C++ modules:

- `original_render_transformed_segments.hpp/.cpp`: decoded the original three transformed packet groups, preserving source record layout and mandatory VU callbacks. Group 0: 32-byte records, 2 allocation units, packet count/pointer offsets +0x10/+0x14. Group 2: 64-byte records, 4 units, +0x20/+0x24. Group 3: 48-byte records, 3 units, +0x28/+0x2C. Original routines: `0x3AF478`, `0x3AF6F8`, `0x3AF530`.
- Group 2 recovers source scalar operations `0x3AF7AC–0x3AF858`: read floats at record +0x34/+0x30, clamp to [0.0010000000474974513, 1.5697963237762451], clamp the second to 0.949999988079071 times the first, call original `0x41D3D8` twice and `0x41D490` once, and write the resulting scalar values at output +0x30/+0x34/+0x38/+0x3C. Original math and VU remain mandatory source-backed callbacks, not guessed host sin/cos or model transforms.
- `original_render_packet_storage.hpp/.cpp`: direct native port of packet reset `0x3AF8B8` (clear bytes 0..59, leaving +0x3C untouched), first-quadword setup `0x3AF8A8`, graphics owner word at +0x38 and original 4-unit backward packet allocation, preserving ownership of each 64-byte header.
- `original_render_model_streams.hpp/.cpp`: original model converter `0x27C614` reads five distinct original pointer pairs, at +0x04/08, +0x14/18, +0x24/28, +0x34/38, +0x44/48, with original record strides 96, 96, 112, 80, and 128 bytes. Actor converter `0x27C520` additionally uses +0x54/58 at 128 bytes per record. Numeric original pointers are never cast to host pointers.

### Original evidence

Original instruction fingerprints, exclusive ends:

- `0x3AF530–0x3AF600`: `fedad04fff370aa0a04de775f25b1b276bdd372723d493b5e046e25f5635d4e7`
- `0x3AF6F8–0x3AF8A8`: `b5a2188e40abe227e04fb5539458d2174a3a0894e5a3ebbd3eff7819f54c6bd6`
- `0x3AF8A8–0x3AF8F0`: `5c2780ae654bc3a47e5cea167ff5b3e3bd49863b4e7e362983c1b4bb2ffcd61e`
- `0x27C520–0x27C614`: `43ff04e5d8ac631b306215e26890db9a971758b57645dad3a5c2bf72d4b43154`
- `0x27C614–0x27C780`: `546c3b911b941eac4fe5d157b070f3f35892a5e20c04b5f6b1d9c7d293a9a760`
- `0x27C8F4–0x27C9E0`: `434d499187d6cdc3553fb78b9dbb8a76875d02931824801eb1cc185509f5f4b1`

Group 2 original `.rodata` float bits at `0x7206D8/DC/E0/E4`: `3FC8EF16`, `3F733333`, `3F800000`, `3A83126F`. The SHA and bit checks are integrated into `native/tools/audit_original_graphics.py`.

## Tests and integration

Three new source-owned test suites: `original_render_transformed_segments_test.cpp`, `original_render_packet_storage_test.cpp`, `original_render_model_streams_test.cpp`. The **complete graphics-focused suite** was compiled using GCC 14.2 `-O2 -Werror` and Clang 17 `-O1 -g -fsanitize=address,undefined -Werror`: **17/17 suites passed under each compiler, 34/34 executions total**. Clang's first batch timed out and was resumed; all runs finished without sanitizer errors. Tests use structural fixtures, not original game assets.

Only the Windows Runtime owner should add new `.cpp` files to the shared `Getchu3Assets` CMake target, register corresponding tests and connect actual stage render ownership. Assets must provide authentic model data and original token-to-buffer mapping; VU0/Physics must help recover `0x3BDF08` and visibility; Graphics must still recover genuine original `0x27C614/0x27C520` per-record conversions and `0x41D3D8/0x41D490` math functions.

**No real level rendered, no Windows D3D11 test, no full CMake/CTest boot, no PS2 emulator or replacement model implementation.** Original packet VU functions and stage/actor source integration remain blockers.
