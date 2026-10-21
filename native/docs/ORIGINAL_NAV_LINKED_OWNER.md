# Original Ape Escape 3 — NavGraph linked-list ownership

This batch reconstructs the native ownership, population and lifecycle of the original NavGraph membership/fallback lists from the actual owner-private USA SCUS_975.01 executable. The retail ELF SHA-256 is 5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63. No original retail executable or navigation assets are published.

## Discovery

The previous recovered 0x2207D0 route dispatcher needed two ordered source polygon lists supplied by the caller. The original game actually uses intrusive circular doubly linked lists:

- Each original link node is 12 guest bytes: +0 next, +4 previous, +8 original polygon reference.
- 0x4E3EB4 allocates a 12-byte self-linked sentinel.
- 0x4E3F40 allocates a new node and writes the source polygon reference at +0x8.
- 0x21F604 allocates the 4-byte list proxy at owner+4, then constructs its sentinel; 0x21F64C and 0x21F690 tear down that list and free its storage.
- 0x4E3DAC traverses sentinel->next around the ring and checks original reference equality.
- 0x21F82C first calls 0x4E3DAC and inserts a node only if absent.
- 0x21F804 forwards original source reference removal to 0x4E3D10, which unlinks matching nodes and frees them.
- 0x4E4154 traverses nodes, runs original cleanup and resets sentinel links to itself.

**Critical original behavior:** both 0x21F82C and the per-polygon 0x21F6D4 insertion path place new nodes BEFORE the old first node. As a result, iterating the circular list yields the reverse of the source insertion order. This matters for nearest-member selection at 0x4E3DEC, because the original uses a strict distance comparison and retains the first member when two are tied.

## Source population

Original 0x21F6D4 scans original NavGraph polygon records in ascending source order. Each polygon's position is obtained through original 0x21B468 centroid calculation, and the game calls 0x21EF3C. Only when the returned original floating value is exactly zero does it check membership and insert that polygon.

Original 0x21EF3C contains a large group of original MIPS multimedia and VU calculations and other engine interactions. Its exact result is NOT reconstructed here. In native production code the predicate remains an explicit, required callback. There is no hardcoded assume-everything-eligible rule.

The native module original_nav_linked_owner.cpp implements:

- OriginalNavCircularList — host-native intrusive sentinel and next/previous links with strong exception-safety during insertion.
- OriginalNavListOwner — original owner+4 creation and the two teardown paths.
- originalNavPopulateList_21f6d4 — authentic ascending source scan, source centroid calculation, exact zero comparison, duplicate suppression, and original head-insertion ordering.
- originalNavRouteDispatchOwned_2207d0 — connects separate source list owners directly into the previously recovered route-request dispatcher, while preserving membership-list versus fallback-list distinction.

The host representation uses C++ pointers and unique_ptr ownership rather than interpreting or re-executing guest pointers. The original 12-byte record layout and links are documented separately.

## Verification

Local private original-ELF audit, using native/tools/verify_original_nav_linked_owner.py, passed:

- 10 complete original function-code windows containing 279 original MIPS instructions
- 50 independent exact opcode sites
- 20 decoded original JAL/J call targets
- Exact owner/constructor/destructor and source-scan addresses

Compiled and tested with g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror. Both UndefinedBehaviorSanitizer and AddressSanitizer+UndefinedBehaviorSanitizer passed. The native regression exercises construction, link consistency, original reverse iteration order, duplicate suppression, removal, clear, source eligibility, two distinct owner lists, nearest fallback, and invalid owner rejection.

A separate private native test read **all 290 original USA .nav files** from the user-supplied retail ISO, constructed the 19,377 genuine original polygon records, and validated circular-list insertion, reverse order, membership, removal and cleanup for every one. The retail survey used a clearly labeled **include-all test predicate** to exercise list structure, because the real original 0x21EF3C predicate is not yet translated. The full 19,377-item inclusion count is NOT a claim that every polygon is eligible in actual gameplay.

## Remaining gameplay blockers

The real original 0x21EF3C predicate, the game-owned live list contents/lifetime, actual actor query vectors, navigation route consumers, Player/Monkey update logic, stage rendering and Windows game startup are still incomplete. This is source-recovered native list infrastructure, not a playable Ape Escape 3 Windows executable. The entire commercial ELF and navigation resources stay private. No GitHub Actions or CI workflows were triggered.
