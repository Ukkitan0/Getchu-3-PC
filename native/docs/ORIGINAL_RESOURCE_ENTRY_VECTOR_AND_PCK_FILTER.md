# Original resource entry selection — Ape Escape 3 USA

Source: owner-private retail SCUS_975.01 and woods_c/exdb.pck, identified by their SHA-256 digests in the accompanying verifier. No game assets are committed.

## Decompiled native behavior

Original function 0x26D24C iterates the source package's 16-byte records and produces a filtered resource list. It reads the source table from owner offset +0x10, source record count from +0x0C, and the string/data bases from +0x08 and +0x00.

For each record, it compares the type string against the requested type using routines 0x42CE60 and 0x42D148. A selected type must be an exact match or have a following NUL or ASCII space. A non-boundary prefix is rejected. The producer passes four words to original 0x26D138: name pointer (+0), type pointer (+4), payload pointer (+8), and payload byte count (+12).

Original 0x26D138 appends the record in source order; 0x26D1B0 clears the current vector range; 0x26D120 computes the count as the byte span shifted right four places; and 0x26D214 addresses an entry by multiplying the index by 16.

The authentic original audio path at 0x35FCBC selects resource type exdb, enumerates this vector, selects bgm_desc.exdb at 0x35FD20 and passes its data pointer and byte count to 0x1BB63C.

## Native code and verification

The native C++20 implementation in original_resource_entry_vector.hpp connects the existing real PCK decoder to the recovered type filter, original-order vector and audio resource selection. It owns copied host strings and payloads, rather than executing original PS2 pointers.

verify_original_resource_entry_vector.py checks nine original ELF regions (267 instructions), 65 opcode anchors, 13 call edges and SHA-pinned owner-private source files. original_resource_entry_vector_test.cpp exercises real woods_c package members, type boundaries, entry order, capacity and reset, ownership after source disposal, and real camera parameters parsed through previously recovered code. Tests passed GCC Release, Clang Release, ASan+UBSan and UBSan locally. The optional CMake test requires AE3_RETAIL_WOODS_C_EXDB_PCK.

A subtle original-ELF detail is confirmed in the verifier: the apparent four bytes spelling PCK at VA 0x6A7D08 are in fact a little-endian function pointer to 0x4B4350 in a vtable. The actual PCK file signature is independently present in the original stage archive; these two appearances must not be conflated.

The full original PCK file-loading allocation path and runtime actor, renderer and Windows boot subsystems remain incomplete. This code is not a PS2 interpreter or a playable game.
