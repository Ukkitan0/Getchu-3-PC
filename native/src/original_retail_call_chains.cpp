#include "ae3/original_retail_call_chains.hpp"
#include <array>
#include <algorithm>
#include <bit>
#include <stdexcept>
namespace ae3 {
void originalMonkeyReplaceAttachment_57173c(OriginalMonkeyOwnedObjects& o,OriginalMonkeyNativeCalls& b){
  // 0x571750..0x571778: optional old virtual[1], then 0x3126E8.
  if(o.attached_at_64)b.call_child_virtual_1(o.attached_at_64);
  o.attached_at_64=b.call_3126e8(o.primary_at_60);
}
void originalMonkeyReplaceAttachment_57178c(OriginalMonkeyOwnedObjects& o,std::uint32_t a,std::uint32_t c,OriginalMonkeyNativeCalls& b){
  // 0x5717B0..0x5717E0: optional old virtual[1], then 0x312760.
  if(o.attached_at_64)b.call_child_virtual_1(o.attached_at_64);
  o.attached_at_64=b.call_312760(o.primary_at_60,a,c);
}
void originalMonkeyRunAttachment_5718cc(const OriginalMonkeyOwnedObjects& o,OriginalMonkeyNativeCalls& b){
  // Original dereferences +0x64 unconditionally. Reject missing original object.
  if(!o.attached_at_64)throw std::runtime_error("Original Monkey 0x5718CC requires real attached object");
  b.call_child_virtual_3(o.attached_at_64);
  b.call_310730(o.primary_at_60);
}
void originalMonkeyForward_571668(void* transform,OriginalMonkeyNativeCalls& b){b.call_3168d0(transform);}
void originalMonkeyForward_5716a8(const OriginalMonkeyOwnedObjects& o,OriginalMonkeyNativeCalls& b){b.call_30fda0(o.primary_at_60);}
bool originalMonkeyTransitionToMode2_20af68(OriginalMonkeyOwnedObjects& o) noexcept{
  // Original 0x20AF68/0x20AF98; Monkey original virtual[4]=0x442028 no-op.
  if(o.source_state_at_4c==2)return false;
  o.source_state_at_4c=2;return true;
}
std::uint32_t originalMonkeyComposeFlags_571e10(std::uint32_t x,std::uint32_t y) noexcept {
  return (x&0xff ? 1u : 0u)|(y&0xff ? 2u : 0u);
}
bool originalMonkeyDispatchVirtual5_316174(void* attached,std::uint32_t flags,
                                            OriginalMonkeyCallGlobals& globals,
                                            OriginalMonkeyLifecycleCalls& backend){
  // Original 0x316188...0x3161A8 changes reentrancy count around
  // the real indirect call to the child vtable slot at +0x14.
  ++globals.active_nested_calls_696c98;
  try {
    const bool result=backend.call_attached_virtual_slot_5(attached,flags);
    --globals.active_nested_calls_696c98;
    return result;
  }catch(...){
    --globals.active_nested_calls_696c98;
    throw;
  }
}
bool originalMonkeyUpdateLifecycle_571904(OriginalMonkeyOwnedObjects& o,
                                          OriginalMonkeyCallGlobals& globals,
                                          OriginalMonkeyLifecycleCalls& b){
  // Original Monkey virtual 0x571904: 0x310890, 0x571E10, 0x316174.
  // Original 0x571E10(0,0) ALWAYS stores zero; prior synthetic scratch
  // values were incorrect. The real 0x316174 virtual-dispatch guard is
  // now reconstructed as native code, not an opaque placeholder.
  b.call_310890(o.primary_at_60);
  const auto originalFlags=originalMonkeyComposeFlags_571e10(0,0);
  if(!originalMonkeyDispatchVirtual5_316174(o.attached_at_64,originalFlags,globals,b)){
    b.call_3108d4(o.primary_at_60);
    return false;
  }
  originalMonkeyTransitionToMode2_20af68(o);
  if(o.attached_at_64)b.call_attached_virtual_1(o.attached_at_64);
  if(o.primary_at_60){b.call_30fc4c(o.primary_at_60);b.call_41a3b0(o.primary_at_60);}
  o.primary_at_60=nullptr;o.attached_at_64=nullptr;
  return true;
}
void originalPlayerForward_349fe4(void* internal,OriginalPlayerNativeCalls& b){b.call_33f240(internal);}
void originalPlayerForward_349ffc(void* internal,OriginalPlayerNativeCalls& b){b.call_33f26c(internal);}
void originalPlayerCleanup_33f240(void* a,void* c,OriginalPlayerCleanupCalls& b){
  // Source calls 0x34310C(internal+0x40), then tail 0x3511D4(internal+0x44).
  b.call_34310c(a);b.call_3511d4(c);
}
OriginalSmallMonkeyAttachment originalSmallMonkeyFactory_3126e8(void* parent,void* field14,OriginalSmallFactoryCalls& b){
  // Original ctor: alloc 8, vptr 0x6B1240, parent +4,
  // call 0x2762BC(parent+0x14,13), call 0x277118(parent+0x14,0,0.2f).
  OriginalSmallMonkeyAttachment obj{parent};
  b.call_2762bc(field14,13);
  b.call_277118(field14,0,0.2f); // actual 0x707100 0x3E4CCCCD
  return obj;
}
std::int32_t originalAddPlayerLives_26fa1c(PlayerProgress& state,std::int32_t amount) noexcept {
  // Original R5900 ADDU (32-bit wrap), PMAXW against 0, PMINW against 99.
  // Data+0x04 is actual PlayerProgress.player_left.
  static_assert(offsetof(PlayerProgress,player_left)==4);
  const auto sum=std::bit_cast<std::int32_t>(std::uint32_t(state.player_left)+std::uint32_t(amount));
  state.player_left=std::clamp(sum,std::int32_t(0),std::int32_t(99));
  return state.player_left;
}
void* originalGameEventGet_29157c(const OriginalGameEventSingleton& g) noexcept {return g.instance;}
void originalGameEventCreate_291588(OriginalGameEventSingleton& g,OriginalGameEventFactoryCalls& b){
  // Source calls 0x41B800(0x18), 0x291560(new), stores global 0x691FE8.
  void* resource=b.call_41b800(0x18);
  if(!resource)throw std::runtime_error("Original event singleton allocation failed");
  b.call_291560(resource);
  g.instance=resource;
}
void originalGameEventDestroy_2915c0(OriginalGameEventSingleton& g,OriginalGameEventFactoryCalls& b){
  // Source calls original free 0x41A3B0 and stores zero to global.
  b.call_41a3b0(g.instance);
  g.instance=nullptr;
}
void originalChipSideEffect_33fd90(PlayerProgress& data,const OriginalGameEventSingleton& g,
                                    OriginalChipSideEffectCalls& b){
  originalAddPlayerLives_26fa1c(data,1);
  void* event=originalGameEventGet_29157c(g);
  b.call_29164c(event,1);
}
void originalChipSideEffect_349b2c(OriginalChipSideEffectCalls& b){
  alignas(16)std::array<std::uint8_t,0x30> scratch{};
  b.call_35ab88(scratch.data());b.call_362178(scratch.data());
}
bool originalChipNotification_2915ec(void* global,OriginalChipSideEffectCalls& b){
  if(!global)return false;
  b.call_2acc6c(global);return true;
}
} // namespace ae3
