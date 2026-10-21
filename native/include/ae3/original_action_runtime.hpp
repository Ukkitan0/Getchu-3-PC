#pragma once
#include <cstddef>
#include <cstdint>

namespace ae3 {
// Names are original ELF object-field offsets, not guessed gameplay terms.
// Native host types use native pointers instead of 32-bit guest memory.
struct OriginalActionFields {
  std::uint32_t flags0c=0;
  void* originalParent00=nullptr;
  void* child20=nullptr;
  std::uint32_t word30=0;
  std::uint32_t word34=0;
  std::uint32_t word38=0;
  std::uint32_t word40=0;
  std::uint32_t parameter14=0;
};
struct OriginalActionChildFields {
  std::uint8_t status45=0;
  std::uint32_t value48=0;
  std::uint32_t word0c=0;
};
// Original global manager slots at 0x696CA0 and 0x696CD0. The native
// structures retain only recovered fields, not a simulated guest memory ABI.
struct OriginalManagerSlot {
  std::uint32_t key00=0;
  std::uint8_t disabled04=0;
  std::uint8_t initialized05=0;
  void* instanceAt08=nullptr;
  // Game fields at +0x10 and +0x20 are still being reconstructed.
};
struct OriginalActionManagers {
  OriginalManagerSlot at_696ca0{};
  OriginalManagerSlot at_696cd0{};
};
class OriginalActionRuntimeCalls {
public:
  virtual ~OriginalActionRuntimeCalls()=default;
  // Actual original callees and virtual slots, not fake success values.
  virtual void* call_221644()=0;
  virtual void* call_factory_virtual2(void* factory,OriginalManagerSlot&)=0;
  virtual bool call_candidate_virtual11(void* candidate,OriginalManagerSlot&)=0;
  virtual void* call_21d898()=0;
  virtual void call_5638f4(void* originalParent,std::uint32_t parameter14,std::uint32_t newValue)=0;
  virtual void call_165220(void* originalOwnedFieldD6)=0;
  virtual void call_26b310(void* originalChannelObject,float originalValue)=0;
  virtual void call_26b524(void* originalChannelObject,std::uint32_t originalValue)=0;
  virtual void call_359e48(void* originalVectorAt50)=0;
};

std::uint8_t originalActionReadStatus_3168ac(const OriginalActionChildFields&) noexcept;
std::uint32_t originalActionReadValue_3168b8(const OriginalActionChildFields&) noexcept;
void originalActionWriteWord_3168c4(OriginalActionChildFields&,std::uint32_t) noexcept;
void originalManagerInitialize_57ba00(OriginalManagerSlot&,OriginalActionRuntimeCalls&);
bool originalActionManagerReady_316fd0(OriginalActionManagers&,OriginalActionRuntimeCalls&);
bool originalActionReady_3102f0(const OriginalActionFields&,const OriginalActionChildFields&,OriginalActionManagers&,OriginalActionRuntimeCalls&);
bool originalActionThreeFlags_310358(const OriginalActionFields&) noexcept;
std::uint32_t originalActionNestedWord_310380(std::uint32_t word70) noexcept;
void originalActionSetValue_3103a8(OriginalActionFields&,std::uint32_t,OriginalActionRuntimeCalls&);
void originalActionForwardOwned_3103f0(void*,OriginalActionRuntimeCalls&);
void originalChannelGuardedForward_276274(bool originalGlobalFlag6354f7,void*,float,OriginalActionRuntimeCalls&);
void originalChannelIntegerForward_2762bc(void*,std::uint32_t,OriginalActionRuntimeCalls&);
void originalVectorForward_276580(void*,OriginalActionRuntimeCalls&);
} // namespace ae3
