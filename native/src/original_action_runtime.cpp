#include "ae3/original_action_runtime.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace ae3 {
std::uint8_t originalActionReadStatus_3168ac(const OriginalActionChildFields& o) noexcept {
  // 3168AC LW parent+0; 3168B4 LBU owner+0x45 in return delay slot.
  return o.status45;
}
std::uint32_t originalActionReadValue_3168b8(const OriginalActionChildFields& o) noexcept {
  // 3168B8 LW parent+0; 3168C0 LW owner+0x48.
  return o.value48;
}
void originalActionWriteWord_3168c4(OriginalActionChildFields& o,std::uint32_t word) noexcept {
  // 3168C4 LW parent+0; 3168CC SW arg to owned +0x0C.
  o.word0c=word;
}
void* originalFactoryGet_221644(const OriginalFactoryGlobal& g) noexcept{return g.factoryAt_647bd0;}
void originalFactorySet_221650(OriginalFactoryGlobal& g,void* ptr) noexcept{g.factoryAt_647bd0=ptr;}
bool originalFactoryIs_221668(const OriginalFactoryGlobal& g,const void* ptr) noexcept{
  return g.factoryAt_647bd0==ptr;
}
void originalManagerInitialize_57ba00(OriginalManagerSlot& slot,const OriginalFactoryGlobal& global,OriginalActionRuntimeCalls& b){
  // 57BA0C: flag+5 is one-time initialization guard.
  if(slot.initialized05)return;
  // 57BA1C clears slot+8 before checking disabled+4.
  slot.instanceAt08=nullptr;
  slot.initialized05=1;
  if(slot.disabled04)return;
  // Original factory acquisition, virtual slot 2 and candidate virtual
  // slot 11: their real engine dependencies must be reconstructed.
  void* factory=originalFactoryGet_221644(global);
  if(!factory)throw std::runtime_error("Original 0x57BA00 requires the real factory-global state 0x647BD0");
  void* candidate=b.call_factory_virtual2(factory,slot);
  if(!candidate)return;
  if(!b.call_candidate_virtual11(candidate,slot))return;
  slot.instanceAt08=b.call_21d898();
}
bool originalActionManagerReady_316fd0(OriginalActionManagers& managers,OriginalActionRuntimeCalls& b){
  // 316FEC initializes first original manager and reads slot+8.
  originalManagerInitialize_57ba00(managers.at_696ca0,managers.factory_647bd0,b);
  if(managers.at_696ca0.instanceAt08)return true;
  // 317008 initializes the second manager ONLY if the first is absent.
  originalManagerInitialize_57ba00(managers.at_696cd0,managers.factory_647bd0,b);
  return managers.at_696cd0.instanceAt08!=nullptr;
}
bool originalActionReady_3102f0(const OriginalActionFields& o,const OriginalActionChildFields& child,
                               OriginalActionManagers& managers,OriginalActionRuntimeCalls& b){
  // 3102FC--310344; the original 3168AC and 316FD0 are ported above.
  if((o.flags0c&1u)==0)return false;
  if(o.word40)return false;
  if(o.word34)return false;
  if(originalActionReadStatus_3168ac(child)==0)return false;
  return originalActionManagerReady_316fd0(managers,b);
}
bool originalActionThreeFlags_310358(const OriginalActionFields& o) noexcept {
  return o.word34!=0 && o.word30!=0 && o.word40==0;
}
std::uint32_t originalActionNestedWord_310380(std::uint32_t word70) noexcept {
  // 310388 loads original parent+0x70 in JR return delay slot.
  return word70;
}
void originalActionSetValue_3103a8(OriginalActionFields& o,std::uint32_t value,OriginalActionRuntimeCalls& b){
  // 3103C4: compare requested with old +0x38. Only changes cause call
  // 5638F4(parent+0, field+0x14, value) and store at +0x38.
  if(value!=o.word38){b.call_5638f4(o.originalParent00,o.parameter14,value);o.word38=value;}
}
void originalActionForwardOwned_3103f0(void* originalOwnedFieldD6,OriginalActionRuntimeCalls& b){
  // 3103F8 loads original parent+0; 310400 adds 0xD6, tail J 165220.
  b.call_165220(originalOwnedFieldD6);
}
void originalChannelGuardedForward_276274(bool originalGlobalFlag6354f7,void* channel,float value,OriginalActionRuntimeCalls& b){
  // 276280 LBU original [0x6354F7], BNEZ skips tail J 26B310.
  if(!originalGlobalFlag6354f7)b.call_26b310(channel,value);
}
void originalChannelIntegerForward_2762bc(void* channel,std::uint32_t value,OriginalActionRuntimeCalls& b){
  // 2762C4 loads original parent+0x1C; tail J 26B524.
  b.call_26b524(channel,value);
}
void originalVectorForward_276580(void* originalVectorAt50,OriginalActionRuntimeCalls& b){
  // 276588 derives original object+0x50; tail J 359E48.
  b.call_359e48(originalVectorAt50);
}
} // namespace ae3
