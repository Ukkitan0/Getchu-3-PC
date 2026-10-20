# Original executable reconstruction — large Player/Monkey lifecycle pass

**October 20, 2026. Source: the owner's hash-verified Ape Escape 3 USA `SCUS_975.01`.** This is not a playable game build and contains no original commercial bytes.

## What was actually recovered

The original PS2 executable's **13 method/call-chain entry points and 25 direct JAL/J edges** have been checked against the machine instructions. Their source-level control flow is now in `native/src/original_retail_call_chains.cpp`, with explicit abstract interfaces for engine functions not yet recovered.

| Original VA | Authentically reconstructed source-level behavior |
|---|---|
| `0x57173C` | Destroy existing Monkey child at +0x64 through its virtual slot 1; create and store child from `0x3126E8` |
| `0x57178C` | Destroy existing child; create and store parameterized child from `0x312760(primary, arg5, arg6)` |
| `0x5718CC` | Invoke child virtual slot 3, then invoke `0x310730` on original primary (+0x60); reject missing child |
| `0x571668` | Tail-forward original primary's +0x20 object to `0x3168D0` |
| `0x5716A8` | Tail-forward original primary at +0x60 to `0x30FDA0` |
| `0x571904` | Full **control-flow** of original Monkey lifecycle branch, detailed below |
| `0x349FE4` | Player member at +0x58 forwarded to `0x33F240` |
| `0x349FFC` | Player member at +0x58 forwarded to `0x33F26C` |
| `0x33F240` | Call `0x34310C` on player internal +0x40, then tail `0x3511D4` on internal +0x44 |
| `0x3126E8` | Original small-object constructor, described below |
| `0x33FD90` | Original game-side reward event sequence: `0x26FA1C(Data,1)`, `0x29157C()`, then `0x29164C(return,1)` |
| `0x349B2C` | Initialize original 0x30-byte scratch and call `0x35AB88` followed by `0x362178` with the same scratch |
| `0x2915EC` | Original conditional notification: if global `0x6AE478` is non-null, call `0x2ACC6C(global)` |

### Real Monkey lifecycle method `0x571904`

This routine processes the actual primary and attached objects. Its original code calls `0x310890(primary)`, creates a stack value using `0x571E10(scratch,0,0)`, then calls `0x316174(attached, first_word_of_scratch)`.

- On false, it calls `0x3108D4(primary)` and keeps both objects.
- On true, it calls `0x20AF68()`, optionally invokes attached virtual slot 1, optionally cleans primary with `0x30FC4C(primary)` and frees it using `0x41A3B0(primary)`, then clears original Monkey fields +0x60/+0x64.

Both paths are represented in native C++, with exact call-order tests. **The underlying calls need their own original source-level implementations** before this routine works with a live Monkey.

### Real small-object constructor `0x3126E8`

The original constructor allocates **8 guest bytes**, writes vtable `0x6B1240` at +0 and parent pointer at +4, then calls `0x2762BC(parent->field14, 13)` and `0x277118(parent->field14, 0, 0.2f)`. The float `0.2f` was verified at original `0x707100` as IEEE-754 `0x3E4CCCCD`. The native representation uses native C++ pointer size rather than emulating the guest ABI.

## Dependency investigation

The audit at `native/tools/audit_original_actor_lifecycle.py`:
1. Requires the **exact** original USA ELF SHA-256.
2. Validates original Player and Monkey virtual tables, selected object field loads/writes, allocation size and float constant.
3. Validates **25** original branch/call sites across **13** method entries.
4. Samples 128 bytes of direct calls for **54** class, factory, helper and event entry points.
5. Finds potential direct incoming J/JAL references throughout original executable sections, without assuming that these are all valid instructions or complete function bodies.

Local private run: **4,843 potential incoming references** across 54 entry points, and 135 sampled outbound direct edges. These are *research leads*, not 4,843 translated functions. The frequently referenced allocator `0x41B800` dominates the count.

To reproduce (metadata only):

```powershell
python native/tools/audit_original_actor_lifecycle.py "C:\path\to\SCUS_975.01" --out "C:\path\to\private-callgraph.json"
```

## Verification and limits

The C++ call chains and both outcomes of `0x571904` passed a local native C++20 optimized build test using `g++ -O2 -Wall -Wextra -Wpedantic -Werror`. The repo also includes CMake/CTest registration. The **Windows** build and complete retail game execution have **not** been verified and GitHub Actions has **not** been dispatched.

Most important remaining source-code targets include `0x310890`, `0x571E10`, `0x316174`, `0x3108D4`, `0x20AF68`, `0x30FC4C`, the Monkey and Player update callers, and the original rendering/game loop. The callback interfaces intentionally have **no fake implementations**.
