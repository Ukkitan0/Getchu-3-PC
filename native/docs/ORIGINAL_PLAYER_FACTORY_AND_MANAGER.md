# Original game engine: Player factory and manager/controller ownership

**Source:** exact Ape Escape 3 USA `SCUS_975.01` SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. This batch is a direct source-level translation of the game executable, not replacement gameplay.

## Main breakthrough: actual Player-to-manager connection

The original global factory at **0x647BD0** is registered with the **original Player object**. The original Player vtable **0x6B3918** has native method **0x34A014** at virtual slot 2. That method compares full **16-byte, zero-padded names**, and returns real Player-owned subobjects:

| Original name | Original return field | Original ELF name location |
|---|---|---|
| `player` | stored pointer at Player +0x5C | `0x7121E0` |
| `main_player` | stored pointer at Player +0x60 | `0x7121F0` |
| `item_chatcher` | embedded subobject at +0x64 | `0x712200` |
| `force` | embedded subobject at +0x180 | `0x712210` |
| `bar_interface` | embedded subobject at +0x70 | `0x712220` |
| `iZone` | embedded subobject at +0x110 | `0x712230` |
| `iHanger` | embedded subobject at +0x188 | `0x712240` |
| `iRope` | embedded subobject at +0x1A0 | `0x712250` |
| `iDashBall` | embedded subobject at +0x1D0 | `0x712260` |
| `iHeroSwitch` | embedded subobject at +0x258 | `0x712270` |

The last six comparisons only execute when real original global pointer getter **0x37D348** returns nonnull (global at `0x6A99BC`). Additional original methods `0x37D354` and `0x37D360` clear that pointer and read byte +0x8A0 when it exists. These functions have been translated too. Missing native Player subobjects remain null and are never faked.

**Original manager initializer `0x57BA00` loads the constant `main_player` from `0x708940`.** It used to call a generic mock factory virtual method in our host code. It now invokes the real recovered `0x34A014` lookup. It still calls the original candidate virtual slot 11 and native `0x21D898` registry selector through explicit unresolved dependencies.

The game sets this Player global during original Player initialization at **0x5B04B4**, and clears it at **0x5B0654 / 0x5B071C** during teardown. Four additional game initialization/teardown callers also use the same setter.

## Connected concrete controller class

The original two allocation wrappers **0x317538** and **0x31759C** both allocate **0x70 / 112 guest bytes** and call original constructors **0x57BA98 / 0x57BAF8**. The newly recovered host-native `OriginalManagerChild70` stores the known original fields, including parent +4, child +0x10, context +0x18, flag +0x1C (signed context[+4] >=2), byte +0x1D, and variant byte +0x30.

Both original constructors call **0x57BCB8**, whose recovered branch, VU0 vector operations, route selection, and original RNG handling are now translated in `native/src/original_manager_initializer.cpp`. The original actor owner/projection effects remain explicitly dependent on genuine game-manager implementations. Variant-one `0x57BAF8` now also preserves the source `+0x20` full 128-bit controller vector before invoking the initializer; variant-zero `0x57BA98` does not write that field. These native constructors still do **not** create a complete live Player/Monkey.

The genuine virtual table **0x6B1820** contains six methods. We translated the first five:

- `0x57BB60` regular destructor and `0x57BBB4` deleting destructor: invoke owned child virtual destructor and, when deleting, original free routine `0x41A3B0`
- `0x57BC0C` query: return 3 if parent +0x40 is set; otherwise call child virtual slot 2 or return 0
- `0x57BC54` forward original arguments to child virtual slot 4, or return its original third argument if no child
- `0x57BC88` invoke child virtual slot 3 if present

The sixth method **0x57C620** has now been audited in full against the verified USA ELF (107 exact instructions), and its source controller-method dispatch and candidate-polygon+0x50 handling are translated in `native/src/original_manager_dispatch_57c620.cpp`. The callee **0x30FFD0** is also translated (51 original instructions) and connected to original source VU0/grid polygon selection. The **0x57C554** type-2 controller child update is source-recovered as an additional 51-instruction native C++ function, including original child constructor/destructor and `0x3161B8` update sequencing. These methods still require original Player/Monkey owner method bodies, actor-owned VU0 and grid operands, exact source diagnostics and real actor-state publication—not a replacement Unreal controller.

## Verified and not yet verified

The owner-private ELF audit `native/tools/verify_original_player_manager_chain.py` passed **93 exact original instruction comparisons, 16 verified direct call/jump targets, two real vtables, and ten exact padded resource names**.

Locally the new Player lookup, child-controller C++, and the corrected manager readiness code passed three native optimized C++20 tests **and** the same three tests with UndefinedBehaviorSanitizer. These are unit tests of reconstructed code, **not** a running retail game. Windows compilation / real game startup have not been verified; no GitHub Actions were triggered.

The original **0x21D898** nearest-polygon calculation is translated and now callable from the mounted stage native bridge. The native original **0x21D46C** stage loader also exposes source-accurate 0x70 polygon records and pending type-1/type-2 descriptors (original record index, action slot, type and argument). This is sufficient to hand the genuine stage data to a future Player/Monkey actor manager, but **not** to implement the still-unrecovered actor spawning semantics by guesswork.

The largest remaining dependencies are the real persistent Player/Monkey factory and owner, real implementations of the original **0x57C620** dispatch's owner callbacks and source actor movement methods, original actor-owned runtime input/route setup and complete update/render/animation integration. The original **0x57BCB8** initializer is translated at source level but is not yet connected to a working live actor owner. The source executable and all game assets stay private.

## Corrected original controller function boundaries and source recovery

Direct source disassembly found **four independent original routines** in
the previously labeled 382-instruction `0x57BF5C..0x57C554` region:

| Original entry | End exclusive | MIPS words | Direct JAL calls | SHA-256 (full method body) |
| --- | --- | ---: | ---: | --- |
| `0x57BF5C` | `0x57C27C` | 200 | 8 | `c625b3ad27bba0b2658f037bc4c5f53dd11096896fe28408eb5fe922e43ae4d4` |
| `0x57C27C` | `0x57C388` | 67 | 4 | `c276349407171b78e83ba5af50aa92c980ced0f638cdf333d670fd3805790b4f` |
| `0x57C388` | `0x57C48C` | 65 | 4 | `c37ced8f115b217f5c79114344cbec86e3745c755bacebc3ff63c87b5614a4fb` |
| `0x57C48C` | `0x57C554` | 50 | 2 | `889e3c33b087048da2eaca86df93c03bb62b4dfbe76baafab3be537540bf609a` |

The original code contains **18** source JAL sites total (8+4+4+2),
including `0x57C3D0 -> 0x57C7CC` in the third function, NOT
in `0x57BF5C`. The earlier single-function attribution was wrong.

The corrected `native/tools/verify_original_manager_movement_57bf5c.py`
SHA-pins all four entrypoint prologues and individual function windows
as well as the previously verified combined region. It also asserts
that original constant `0x708A48` contains the packed method pair
`{0x57C388, 0}`, written by source routine `0x57C27C`.

### Newly translated complete original 0x57C27C function

`native/src/original_manager_route_child_57c27c.cpp` implements the
real 67-instruction route-child controller update with source-specific
callbacks for the still-owner-dependent methods `0x3176AC`,
`0x30C1D4`, the child deleting destructor, `0x3161B8`, and
`0x316174`. It reproduces:

- Source readiness `0x3176AC` checked only when child +0x10 is null.
- False readiness writes method pair `{0x57C388,0}` and immediately
  delegates to original `0x316174`, skipping child creation/update.
- True readiness calls original `0x30C1D4`, rechecks the child
  pointer after the callback, deletes a reentrantly-installed original
  child via virtual slot 1, then stores the new genuine child.
- `0x3161B8` zero result returns zero without changing the original
  method pair or invoking `0x316174`.
- Successful child update installs `{0x57C388,0}` and forwards the
  real source return from `0x316174`; unimplemented owner callbacks
  propagate failure, never claim a synthetic update.

The independent `OriginalRouteChildController57C27C` CTest was
compiled locally with **GCC and Clang** using C++20,
`-O2 -Wall -Wextra -Werror -pedantic -fsanitize=undefined` and
passed on both. A separate direct audit against the exact owner-private
USA executable passed all four function region hashes, prologues,
method constant, and each 8/4/4/2 original JAL count.

### Original 0x57C388 route-to-child and 0x57C48C follow-up

`native/src/original_manager_routed_update_57c388.cpp` recovers
the 65-instruction controller method that precedes the source
`0x57C48C` follow-up. It captures genuine parent+0x14->+0x40
packed source vector bytes BEFORE invoking recovered original
`0x57C7CC` (when source flag+0x1C is set). Actual VU0 transforms
and original `0x30C21C` child construction remain mandatory game-owner
callbacks, not invented native steering. It rechecks/destroys an
existing child+0x10 after source construction, calls real `0x3161B8`,
then publishes retail `0x708A38` pair `{0x57C48C,0}` only after a
successful source update and delegates to `0x316174`.

`native/src/original_manager_followup_57c48c.cpp` recovers the
full 50-instruction next-method body with original `0x30C520`
creation, child virtual destruction, `0x3161B8` update and the
shared packed `PCEQW/PPACB` vector comparison. Missing actual
parent and actor VU0 owner state causes the callback to reject.
Both C++20 regression targets passed GCC and Clang `-O2` with
UndefinedBehaviorSanitizer:

- `OriginalControllerRouted57C388`
- `OriginalControllerFollowup57C48C`

The two source constants are now individually verified:
`0x708A48 = {0x57C388,0}` and
`0x708A38 = {0x57C48C,0}`.

### Corrected original type-two 0x57C554 return semantics

The previous port inverted this method's final result. Direct
USA disassembly confirms original `0x57C5CC` variant-zero branch
falls through `0x57C604` LW and returns source controller field
`+0x14`, **not** constant one. With variant-one and successful
four-lane packed equality, the result is also original `+0x14`;
inequality returns constant one through the `0x57C600`
branch-delay slot. Source `0x57C48C` uses the same packed
comparison/return tail with matching instructions. Native code,
tests, source comments and the ELF verifier now reflect these
verified source returns; zero-valued real `+0x14` is not replaced
with a synthetic boolean success.

**Still not recovered in full:** the original owner-specific VU0
register calculations and input provenance inside `0x57BF5C`,
the actual source owner methods `0x3176AC`, `0x30C1D4`,
`0x30C21C`, `0x30C520`, `0x30C57C`, `0x36F5F4`,
real
implementations of vtable[5] for every game object, original
`0x21DB64` diagnostics, and persistent Player/Monkey lifetime.
The original `0x316174` and `0x3161B8` wrapper bodies are now
natively recovered but still need authentic owner virtual methods. Full Windows/Unreal Editor compilation has
not been verified; no proprietary ELF or ISO was committed.


### Integrated native original controller-method dispatch

`native/src/original_manager_native_method_router.cpp` now
connects the source-recovered real **0x57C620** dispatcher to
the original direct-method VAs:

- `0x57C27C` — original readiness, child creation and next-method assignment
- `0x57C388` — original route update, VU0-dependent child creation, method assignment
- `0x57C48C` — original follow-up child update and packed comparison
- `0x57C554` — original type-two child update and packed comparison

The native router passes the real original `a1` argument to the
C++ recovered method implementations and delegates genuine
`0x30FFD0` polygon adoption and source diagnostic globals to the
actual game-owner callbacks. Original method source
`+0x08` addresses are used ONLY as explicitly audited dispatch
**identifiers**, never converted into Windows function pointers.

Tagged original virtual dispatch and nonzero source-subobject
offsets, as well as unrecovered original methods (including
`0x57BF5C`), remain delegated to a mandatory real owner. An
owner without the genuine implementation **must reject** them;
no synthetic actor movement is performed.

The native CTest `OriginalRecoveredControllerMethodDispatchIntegration`
covers `0x57C620` -> `0x57C27C` -> `0x57C388` ->
`0x57C48C`, original type-two `0x57C554`, real
`0x30FFD0` callback forwarding, virtual-method refusal,
unavailable method refusal and signed source-this-offset refusal.

The native C++20 tests for `0x57C27C`, `0x57C388`,
`0x57C48C`, corrected `0x57C554`, and the integrated method
router were compiled and executed locally under BOTH GCC and
Clang with UndefinedBehaviorSanitizer. The corrected original
four-function verification hashes, MMI compare opcode pairs,
two retail next-method metadata constants and direct JAL sites
also matched the private USA ELF exactly.

This makes original controller **dispatch and branch sequencing**
more complete. It does **not** supply the still-needed original
game-owner allocator methods, actor-specific VU0 operands,
per-frame behavior, full Player/Monkey object lifecycle, native
renderer or retail-stage gameplay. The complete Windows native
DLL, CTest suite and Unreal Editor build have not been verified.


### Original 0x57BF5C outer movement/child controller recovered

`native/src/original_manager_movement_57bf5c.cpp` now translates
the original **200-instruction method's branch decisions and
source child-update sequencing**, using the verified USA executable.
Its owner-provided inputs are **mandatory** for original VU0 state
and original game methods; these aren't replaced with host physics:

- Original `C.OLT.S` direction test against source scalar
  `0x708A40` (exact bits `0xBF34FDF4`), transitioning directly
  to original `{0x57C27C,0}` when less.
- Source `0x21B854` projection invoked only along the original
  controller flag+0x1C branch, followed by the original VU0
  metric/SQRT result and strict comparison to genuine
  `0x708A44 = 2.0f`. Eligible early transitions go to
  `{0x57C27C,0}`.
- Original calls `0x441F40` twice, now confirmed as a genuine
  **no-op** by its complete 12-word ELF window; no external
  initializer or invented contents are needed. Then genuine
  `0x36F5F4` fills source-owned scratch and the actual
  `0x30C57C` child constructor runs. A child published during
  synchronous construction is deleted through its original
  virtual slot 1 before the new child is installed.
- Original `0x3161B8` update gate; zero means zero return and
  conditional original `0x21DB64` diagnostics, only when
  actual `0x6354E7` is nonzero and `0x6354E0` is zero.
- Successful update dispatches original `word14 != 2` to
  `{0x57C27C,0}`, or `word14 == 2 && flag1c` to
  `{0x57C48C,0}`, each invoking actual `0x316174`.
  Without flag+0x1C, original variant zero returns 2;
  variant one returns **2** for four-lane source equality,
  **1** for mismatch using the source `BEQZL/MOVZ` tail.

The existing `originalManagerDispatchNativeMethods_57c620`
native router now recognizes original method VA `0x57BF5C`
and dispatches it when an explicit real movement source owner
is installed. No owner means mandatory fallback to the real
game-manager callback, which must reject unsupported methods.

The corrected original verification script checks 18 exact
original movement branch/opcode sites, original PCEQW/MOVZ
instructions, the real `0x708A30..0x708A44` method/scalar
constants, all four method-body SHA-256 windows and their
source JAL edges against SHA-pinned retail `SCUS_975.01`.

`OriginalMovementController57BF5C` was registered as a native
CTest target and includes original early-transition, projection,
metric, child creation, update, variant equality and diagnostic
branches. Local isolated C++20 smoke tests and prior native
dispatcher integration tests both compiled and passed under
**GCC and Clang with UndefinedBehaviorSanitizer**. This does
**not** imply the new full repository CTest, Windows DLL, Unreal
Editor or the actual original game has been booted successfully.

**Remaining true-game integration:** the original actor owner
must supply the actual VU0 state and original child/
animation/movement method bodies; a persistent retail
Player/Monkey manager must drive this native controller
through a complete frame with real decoded game assets.


### Recovered original common child update (0x316174 / 0x3161B8)

The independently hash-pinned 17-word `0x316174` function and
29-word `0x3161B8` function are translated in
`native/src/original_manager_update_316174.cpp`.
These update **all** original child types through their true
vtable[5] methods; their native owner callbacks cannot be replaced
with dummy actors or unconditional success.

This source recovery verifies the global update-depth word at
`0x696C98`, original flag masking of a2/a3, the vtable slot-5
call, the full status write at controller+0x14, original child
virtual deleting destructor slot 1 and clearing controller+0x10.
Native C++ returns the true completed/active status and
rejects missing source child objects.

The source ELF verifier independently pins both full function
hashes and 23 original instruction sites. The standalone
`OriginalControllerUpdate3161743161B8` regression and the
connected `0x3161B8 -> 0x316174 -> 0x57C620` original
controller dispatch smoke path passed under GCC and Clang
with UndefinedBehaviorSanitizer. The full Windows and Unreal
project build and actual original game ownership are still
outstanding. See
`native/docs/ORIGINAL_MANAGER_UPDATE_316174.md`.


### Proven source 0x441F40 no-op and 0x36F5F4 frame extent

The exact original `0x441F40..0x441F70` is 12 MIPS
instructions (SHA-256
`4fd423015f167deede14f0a830e898e2e308de5aed3c8974fca56d6e62b84e96`).
Its only operations are GPR loop bookkeeping, five NOPs, and return;
there is **no store** to either scratch object. The former native
interface incorrectly required two game-owner initializer callbacks,
which have been removed.

At `0x57C148`, actual original `0x36F5F4` receives
`stack+0x20` as `a0`. Five independent verified source instructions
write to **`a0+0xA0`, `+0xA4`, `+0xA8`, `+0xAC`**.
The former two-0x40-array native representation was too short and
would hide those genuine outputs. The corrected native
`OriginalMovementScratch_57bf5c` is a full contiguous
`0x110`-byte source-frame object, with compile-time assertions
on offsets `+0x40` and `+0x80`, and a complete-object byte view
allowing original `+0xAC` writes without pointer arithmetic past
one 0x40 subarray.

The `0x57BF5C` host regression now verifies original +0xA0/+0xAC
writes supplied by the source `0x36F5F4` callback before genuine
`0x30C57C` actor construction. Local standalone movement and
connected dispatcher builds passed GCC and Clang with
UndefinedBehaviorSanitizer after this correction. The real
`0x36F5F4` VU0 and runtime child-construction bodies are still
not translated or provided by this source-frame layout alone.


### Native original PCEQW comparison now shared across three actor methods

`native/include/ae3/original_manager_packed_compare.hpp`
provides one source-specific implementation of the full **four-word
128-bit integer comparison** used by the already recovered original
controller `0x57C1E0`, `0x57C508`, and `0x57C5D4` tails.

The implementation deliberately compares original **raw 32-bit SQ
lane patterns**, not IEEE-754 floating values. Signed positive/negative
zero and NaN payloads retain original bit identity; there is no
floating approximation, vector epsilon or host-synthesized input.

`OriginalManagerChild70` retains two separately provenance-checked
optional original 16-byte values: `controller20` (written by original
variant-one `0x57BAF8`) and `controller40` (written in the source
type-two `0x57BED4` SQC2 delay slot or later by an authentic original
actor owner). The native `originalManagerInitialize_57bcb8` now
preserves this genuine +0x40 SQC2 value when the type-two branch is
selected; it does not guess a +0x40 vector in other branches.

The three source controller owner callback interfaces have a native
default packed-vector implementation. Existing explicit owner callback
overrides remain compatible. When either real operand has not been
provided, the default raises `std::invalid_argument`, rather than
claiming a false movement/status result. Non-vector functions,
including original `0x36F5F4` owner VU0 work and actual actor
construction, are NOT implemented by this change.

Regression `OriginalControllerPackedPCEQW128Bit` exercises each
source byte of all four PCEQW lanes, signed zeros, fixed NaN
payloads, missing operand refusal, and the actual return branches
of `0x57BF5C`, `0x57C48C` and `0x57C554` when no owner overrides
the native comparison. Constructor and initializer regressions
add exact +0x40 source-vector preservation and publication checks.

A standalone bitwise comparison smoke harness passed GCC and
Clang C++20 under UndefinedBehaviorSanitizer in this work session.
This is not a claim of running the complete new CTest target,
Windows DLL, Unreal Editor, or game itself. The owner-private
original executable was not mounted in this current environment,
so no new retail-machine-code claims are based on fresh local ELF
verification.


### Original 0x36F5F4 / 0x30C57C native source recovery (USA ELF reavailable)

The original pinned `SCUS_975.01` executable was retrieved
from the user's **private prior handoff archive** and verified
byte-for-byte against project SHA-256; it is NOT being copied to
GitHub. This restored actual retail machine-code inspection for
the movement scratch and child constructor chain.

The first entry/return window of original `0x36F5F4`
is 58 instructions and contains real VU0 math and five
cross-entry branch destinations. Its exact scalar/store tail
at `0x36F6A8..0x36F6D8` is now translated for the actual
`0x57C148` caller (`t0=0`, source 32-bit EE stack
pointer). It sets original scratch `+0xA0` to the source
runtime BSS address `0x744DD0`, `+0xA8/+0xAC` to zero
and `+0xA4` to 1.0f only on original ordered
`f1 < 1.0f`; otherwise +0xA4 stays zero.
Missing actual VU0 `f1` is NOT guessed.

The immediately following original `0x30C57C..0x30C670`
function **has now been completely recovered at the original
data-initialization level**: source allocation size 0xD0,
original vtable VA 0x006B0B20, guest parent pointer at +0x04,
two low-byte parameters +0x08/+0x09, exact 0xB0-byte source
scratch copy at +0x10..+0xBF, and +0xC0 cleared.
The native typed owner preserves the untouched original
allocation bytes as *unknown*, not synthesized zeros.

New native CTest targets
`OriginalMovementScratchTail36F6A8` and
`OriginalMovementChild30C57CSourceCopy` are linked in the
Unreal native bridge CMake. The latter verifies that the source
0x36F5F4 scratch-tail results survive the complete original
0x30C57C constructor copy and source scratch release.
GCC and Clang C++20 UndefinedBehaviorSanitizer isolated
compilation and regressions passed for both. The genuine ELF
verified **all 61 constructor words**, its callsite and 28
specific opcode sites; it also verified the source movement
entry window and runtime-BSS placement.

**Still required:** original 0x36F5F4 complete VU0 vector
computations and cross-entry branches, original 0x30C57C
vtable **methods and ownership integration**, real actor
lifecycle/destruction, persistent Player/Monkey creation,
stage movement, rendering/animation and original game loop.
The native constructor data translation by itself is NOT a
live original actor. The full Windows DLL, Unreal Editor
build and playable game have not been verified here.

See `native/docs/ORIGINAL_MOVEMENT_36F5F4_RECOVERY.md` and
`native/docs/ORIGINAL_MOVEMENT_30C57C_CONSTRUCTOR.md`.


### Original 0x36F5F4 VU0 operand and vector math advanced (USA source pinned)

The original `0x36F5F4..0x36F698` source VU0 math is
now recovered for its ordinary-float movement-call operand path:
genuine original EE A1/A2/A3 packed vectors enter VSUB, VMUL,
VADDy, VADDz and produce the exact **separate source VF1.x
and VF2.x squared XYZ distances** used in the two original
COP1 comparisons. The second VF2.x scalar feeds the verified
`0x36F6A8` original source scratch tail and then the already
recovered `0x30C57C` 0xD0-byte child copy.

The original first-COP1-comparison branch now also has a native
source translation: first squared distance below 1.0f selects
the original VF4 midpoint of A1 and A3; otherwise VF4
retains A2. Original PPACW zero t5 register packing establishes
VF7=0 before the source final VF3/VF1/VF2 subtractions.
The result preserves original full VF1/VF2/VF3/VF4 vector
values rather than replacing the movement with host-steering.

This source translation accepts only ordinary finite floating-point
operands where the PS2's special status, denormal and overflow
semantics need not be guessed; the rest are refused.
The source original callsite `0x57C148` and the vector
opcodes were verified against the user's private real
USA executable, which is not committed to GitHub.

The newly registered CTests are
`OriginalMovementVU0SourceScalar36F5F4`,
`OriginalMovementVU0SourceVectors36F5F4`, and
`OriginalMovementVectorScratchChildChain`. Local
GCC and Clang C++20 UndefinedBehaviorSanitizer tests
passed for these three native code paths. The complete
repo-wide CTest, Windows DLL and Unreal Editor remain
unverified; no playable game is claimed.

**Still blocking actual source actor control:** retail packed
operand ownership into this VU0 function, PS2 hardware-specific
edge-case behavior, other 0x36F5F4 entry branches, real
0x30C57C child vtable/lifetime, original animation and
complete Player/Monkey stage/game runtime.


### Original child source-state provenance and flag-driven lifetime

The original 0x36F5F4 first-return VU0 calculations now preserve
VF1 through VF7 and publish only their **proven** source scratch
writes (+0xA0..+0xAF). The 0x30C57C original 0xD0-byte child
constructor now has a provenance-aware source-copy overload,
preventing the 0xB0 copied bytes of unknown original stack
state from appearing as invented zero-valued actor data.

The original child vtable **0x6B0B20**, its slot5
**0x557E30** and deleting destructor slot1 **0x556A0C**
were independently recovered from the pinned USA ELF.
The native child lifecycle supports the exact original
bit0/bit1 completion status1, required parent+0x14
delay-slot read, and original 0x3161B8 controller status
publication and source child deletion. Other original
0x557EB4+ normal gameplay branches remain deliberately
unimplemented, as do the live persistent Player/Monkey
owner and full native game loop.


### Genuine normal child-update completion (0x36F954 / 0x557E30)

The recovered original movement-child data now participates
in a real **zero-flag normal update completion path**, not
only the prior special flag-driven early exit. The exact
`0x36F954` source return gate reads child status `+0xB4`
(previously written to original scratch `+0xA4`
by `0x36F5F4`). For the verified original
`0x557EC8` caller, `f12=2.0`; when child status
is at least 1.0, the original `0x36F954` returns 1,
the genuine child slot5 `0x557E30` returns 2,
and native `0x3161B8` destroys the completed child.

Two source-hash-pinned branch windows and 26 instruction
sites have been independently verified against the
owner-private USA executable. GCC/Clang UBSan tests
exercise the direct source branch and a complete
VU0-vector → scratch → child construction →
normal completion/deletion chain.

For status below 1.0, nonzero child flag `+0xC0`,
or unknown source status bytes, normal movement/collision
continues through still-unrecovered source functions.
These paths continue to refuse rather than pretending
the actor moved or completed. The actual per-frame
Player/Monkey gameplay owner and Windows/Unreal runtime
remain incomplete.


### Original nonterminal child route setup progressed

The previously blocked normal `0x557E30` update (source
`0x36F954` returns zero for nonterminal status) now has a
real source-backed `0x557ED8..0x557F30` setup translation,
invoked only when actual game-owner methods are supplied.

The original `0x317B9C` parent resource selector has been
fully translated across all four source method branches
(parent+0x10/+0x34/+0x40, child mode low8), preserving
32-bit guest VA identity, branch-likely conditional loads
and no invented runtime resources. The verified setup writes
original child+0xB4=0 and +0xB8=0, invokes genuine
`0x276B68` and conditionally `0x2767F4`, always invokes
`0x2773D4`, and then sets child+0xC0=1. It leaves
real `0x557F34/0x36FB18` animation/collision continuation
unrecovered, never reporting a completed actor frame.

Additionally original `0x317B34` resource float source
returns from parent+0x08 table offsets +0x374 (kind1)
or +0x3B4 (mode zero) are now natively recovered;
the nonzero-mode VU0 path still requires real math.
See `native/docs/ORIGINAL_MOVEMENT_NORMAL_SETUP_557ED8.md`.

New native regression targets and original ELF hash/opcode
verifiers cover these behaviors; GCC and Clang isolated
sanitizer checks passed. Full Windows/Unreal build and
playable game are **not** verified.


### Genuine resource metric and persistent child re-entry expanded

Recovered original `0x317B34` now includes the
nonzero-mode VF1 VU0 length-squared, source COP1
SQRT/ADD and capped MIN.S branch for **exactly
reproducible ordinary source operands**, in addition
to the two previously recovered direct resource-float
returns. The calculation publishes actual modified
VF1 register lanes to the game owner, avoiding
stale source vector state.

Original `0x557F34..0x557F50` obtains resource
metric f0 from this function, reads real parent+0x70,
multiplies original f12 in the `0x36FB18` JAL
delay slot, and dispatches the genuine movement step.
The native `0x36FB18` implementation covers
only the source **status-complete** or **nonpositive
movement-scale** fast returns. Positive movement below
threshold requires the actual game's collision methods.

The original initialized-child `0x557EB4 BNEZL`
branch now skips first-time `0x557ED8` setup
when child+0xC0 is set; it enters `0x557F34` directly
on subsequent updates. Local GCC and Clang sanitizer
integration tests validated source control-flow
reentry and owner callback requirements. Further
`0x557F54+` child animation/collision and
persistent Player/Monkey runtime remain incomplete.
See `native/docs/ORIGINAL_MOVEMENT_METRIC_557F34.md`.


### Original 0x36FB5C flagged positive movement advanced

The actual `0x36FB5C..0x36FC28` 51-instruction
original retail USA movement window (SHA-256
`eba23d30f1f7b8186abebf22a802722e6835be500a5286906420fb02c943c457`)
is now recovered for child `+0xB8` bit1-set,
untagged source address, **exact representable ordinary
binary32 inputs**. The calculation uses the genuine
VF1 register, original VMUL/VADDy/VADDz and
0.125f coefficient, and translates original COP1
SQRT, MUL, DIV, ADD, threshold and overshoot behavior.

A completed single original flagged pass publishes
the changed VF1 and updates typed child
`+0xB4/+0xB8` status/flags. A pending source loop
into `0x36F8C8` is explicitly represented as
**not completed**, with no invented result. The
persistent owner may opt into this genuine path
using source VF1/guest data or delegate to its
full original method. Typed mutation requires
recomputing and verifying the original pass,
rejecting stale or fabricated results.

Source data: `native/docs/ORIGINAL_MOVEMENT_FLAGGED_36FB5C.md`.
Original `0x36F8C8` VF1 regeneration and the
rest of `0x557F54+` actor collision/animation
remain unresolved, so this does not establish
a full playable Player/Monkey game update.
