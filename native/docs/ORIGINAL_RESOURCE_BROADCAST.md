# Ape Escape 3 USA — original 128-bucket resource dispatcher

Source: original owner-private USA retail SCUS_975.01, SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Original binary and retail resources are never committed.

## Recovered original caller and virtual dispatch chain

Original 0x21F8D0 passes the navrange method table at 0x6AE248 to
0x27242C. That wrapper constructs a local event descriptor at stack
offset 0 with vtable 0x6AF610 and a receiver argument at stack +4.

**0x27244C calls 0x36D844**, passing a pointer to the source
128-bucket table at the original container's +4 offset. The complete
0x36D844..0x36D8C4 implementation reads bucket heads sequentially
for indices 0..127 and traverses singly linked source nodes.
Each node stores its payload at **+0x20** and next link at **+0x24**.
The original fetches next after the callback returns.

It invokes descriptor.vtable[0] = 0x518E4C with the node and
resource pointer. That wrapper calls descriptor.vtable[3] =
0x518E24, which dereferences the underlying receiver pointer
at descriptor +4 and calls receiver.vtable[0] with the source
resource payload. For an installed navrange receiver, this is
the original 0x4E4060 handler, which checks the 'navrange'
resource magic before creating keyed owners.

After the broadcast, original 0x27242C assigns a transient
0x6AF628 vtable, then invokes 0x4E3D9C. **Correction to previous
notes:** 0x4E3D9C only writes the base vtable 0x6AE290 to
the object. It does NOT free or destruct that object.
Original navigation-stage map allocation/free is handled by
separate calls inside 0x21F940. The existing native
registration hook has therefore been renamed to
`originalResetDescriptorVtable_4e3d9c`.

## Native implementation

`native/include/ae3/original_resource_broadcast.hpp` contains:
- Original-order 128-bucket native array and linked resource records.
- `originalResourceBroadcast_36d844` to iterate and dispatch
  each payload through a required native receiver callback.
- `originalResourceDispatch_27242c` to reproduce the
  stack-descriptor's observable resource delivery.
- `originalNavrangeDispatchRegistered_27242c` to pass
  already-decoded resources into the existing source-derived F20
  polygon admission and keyed-owner registry.

Native pointers replace the source's 32-bit guest addresses.
The source VA constants are data identifiers, never host
function pointers. Node ownership and game resource allocations
remain external; the code is **not** an EE interpreter or a
substitute resource archive loader.

## Verification

`native/tools/verify_original_resource_broadcast.py`
pins the private retail ELF SHA, all **five entire original
instruction regions (74 words)**, **36 exact opcode anchors**,
two original four-method tables, relevant branch destinations,
128 buckets, +0x20/+0x24 payload and next offsets, and the
two-step virtual invocation chain.

The native regression covers empty buckets; buckets 0, 1, 127;
multiple nodes per bucket; source-compatible reading of the next
link after a callback; null-resource rejection; unrelated resource
magic; full navrange polygon admission through the new broadcast;
duplicate list suppression; and owner teardown. The previous
native registration tests are rerun after correcting the hook name.

Locally passed optimized strict C++20, UBSan and ASan+UBSan
compilation and execution with a test-only stand-in for still
unavailable engine registry implementation. The complete CMake
Windows game build was not executed in this pass.

## Unfinished

Resource-bucket population/allocation from real source media,
the actual generic receiver callback object lifetime, precise
retail VU0 floating point, live stage dispatch, actor update,
rendering, and game boot. The private ELF/Lua archive does not
contain the complete original ISO resources, and the earlier
1,706-archive survey did not find a real navrange allocation.
**No playable native Windows build yet.** No GitHub Actions
or hosted builds were started.
