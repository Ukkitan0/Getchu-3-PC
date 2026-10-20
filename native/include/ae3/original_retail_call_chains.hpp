#pragma once
#include <cstdint>
#include "ae3/player_state.hpp"
namespace ae3 {
struct OriginalMonkeyOwnedObjects {
  void* primary_at_60=nullptr;
  void* attached_at_64=nullptr;
  std::uint32_t source_state_at_4c=0;
};
class OriginalMonkeyNativeCalls {
public:
  virtual ~OriginalMonkeyNativeCalls()=default;
  virtual void call_child_virtual_1(void*)=0;
  virtual void* call_3126e8(void*)=0;
  virtual void* call_312760(void*,std::uint32_t,std::uint32_t)=0;
  virtual void call_child_virtual_3(void*)=0;
  virtual void call_310730(void*)=0;
  virtual void call_3168d0(void*)=0;
  virtual void call_30fda0(void*)=0;
};
// These are source-level control-flow translations from original USA ELF,
// not synthetic monkey gameplay. Callee implementations remain mandatory.
void originalMonkeyReplaceAttachment_57173c(OriginalMonkeyOwnedObjects&,OriginalMonkeyNativeCalls&);
void originalMonkeyReplaceAttachment_57178c(OriginalMonkeyOwnedObjects&,std::uint32_t,std::uint32_t,OriginalMonkeyNativeCalls&);
void originalMonkeyRunAttachment_5718cc(const OriginalMonkeyOwnedObjects&,OriginalMonkeyNativeCalls&);
void originalMonkeyForward_571668(void*,OriginalMonkeyNativeCalls&);
void originalMonkeyForward_5716a8(const OriginalMonkeyOwnedObjects&,OriginalMonkeyNativeCalls&);

class OriginalMonkeyLifecycleCalls {
public:
  virtual ~OriginalMonkeyLifecycleCalls()=default;
  virtual void call_310890(void*)=0;
  virtual bool call_attached_virtual_slot_5(void*,std::uint32_t)=0;
  virtual void call_3108d4(void*)=0;
  virtual void call_attached_virtual_1(void*)=0;
  virtual void call_30fc4c(void*)=0;
  virtual void call_41a3b0(void*)=0;
};
// True = original branch released primary and attached objects;
// false = original normal processing path. No PS2 CPU/memory is emulated.
struct OriginalMonkeyCallGlobals {std::uint32_t active_nested_calls_696c98=0;};
// Original 0x571E10: low-byte booleans packed into bits 0 and 1.
std::uint32_t originalMonkeyComposeFlags_571e10(std::uint32_t,std::uint32_t) noexcept;
// Original 0x316174: increment global 0x696C98, invoke child virtual[5],
// then decrement the global, returning the actual child result.
bool originalMonkeyDispatchVirtual5_316174(void*,std::uint32_t,OriginalMonkeyCallGlobals&,OriginalMonkeyLifecycleCalls&);
bool originalMonkeyUpdateLifecycle_571904(OriginalMonkeyOwnedObjects&,OriginalMonkeyCallGlobals&,OriginalMonkeyLifecycleCalls&);
// Original 0x20AF68: state +0x4C becomes 2. Monkey virtual slot 4
// is verified real no-op 0x442028; no fabricated game behavior.
bool originalMonkeyTransitionToMode2_20af68(OriginalMonkeyOwnedObjects&) noexcept;

class OriginalPlayerNativeCalls {
public:
  virtual ~OriginalPlayerNativeCalls()=default;
  virtual void call_33f240(void*)=0;
  virtual void call_33f26c(void*)=0;
};
void originalPlayerForward_349fe4(void*,OriginalPlayerNativeCalls&);
void originalPlayerForward_349ffc(void*,OriginalPlayerNativeCalls&);
class OriginalPlayerCleanupCalls {
public:
  virtual ~OriginalPlayerCleanupCalls()=default;
  virtual void call_34310c(void*)=0;
  virtual void call_3511d4(void*)=0;
};
void originalPlayerCleanup_33f240(void*,void*,OriginalPlayerCleanupCalls&);

// Original 0x3126E8 allocates an 8-byte PS2 object, gives it vtable
// 0x6B1240 and stores owner at +4. On Windows pointers have native size.
struct OriginalSmallMonkeyAttachment {
  static constexpr std::uint32_t original_vtable_va=0x6b1240;
  void* parent=nullptr;
};
class OriginalSmallFactoryCalls {
public:
  virtual ~OriginalSmallFactoryCalls()=default;
  virtual void call_2762bc(void*,std::uint32_t)=0;
  virtual void call_277118(void*,std::uint32_t,float)=0;
};
OriginalSmallMonkeyAttachment originalSmallMonkeyFactory_3126e8(void*,void*,OriginalSmallFactoryCalls&);

// Original 0x26FA1C: Data+0x04 32-bit ADDU, PMAXW 0, PMINW 99.
std::int32_t originalAddPlayerLives_26fa1c(PlayerProgress&,std::int32_t) noexcept;
// Original event global 0x691FE8: getter 0x29157C, allocator/initializer
// 0x291588, and deallocator/clearer 0x2915C0.
struct OriginalGameEventSingleton {void* instance=nullptr;};
void* originalGameEventGet_29157c(const OriginalGameEventSingleton&) noexcept;
class OriginalGameEventFactoryCalls {
public:
  virtual ~OriginalGameEventFactoryCalls()=default;
  virtual void* call_41b800(std::uint32_t original_size)=0;
  virtual void call_291560(void* original_new_object)=0;
  virtual void call_41a3b0(void* original_object)=0;
};
void originalGameEventCreate_291588(OriginalGameEventSingleton&,OriginalGameEventFactoryCalls&);
void originalGameEventDestroy_2915c0(OriginalGameEventSingleton&,OriginalGameEventFactoryCalls&);

class OriginalChipSideEffectCalls {
public:
  virtual ~OriginalChipSideEffectCalls()=default;
  virtual void call_29164c(void*,std::uint32_t)=0;
  virtual void call_35ab88(void*)=0;
  virtual void call_362178(void*)=0;
  virtual void call_2acc6c(void*)=0;
};
void originalChipSideEffect_33fd90(PlayerProgress&,const OriginalGameEventSingleton&,OriginalChipSideEffectCalls&);
void originalChipSideEffect_349b2c(OriginalChipSideEffectCalls&);
bool originalChipNotification_2915ec(void*,OriginalChipSideEffectCalls&);
} // namespace ae3
