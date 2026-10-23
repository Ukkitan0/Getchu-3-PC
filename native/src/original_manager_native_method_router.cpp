#include "ae3/original_manager_native_method_router.hpp"
namespace ae3 {
namespace {
class ActualMethodCalls final:public OriginalManagerDispatchCalls_57c620 {
public:
    ActualMethodCalls(OriginalManagerChild70& controller,std::uint32_t a1,
                      OriginalManagerNativeMethodOwner& originals)
      : controller_(controller),a1_(a1),originals_(originals) {}
    std::uint32_t call_original_direct_57c7ac(
        std::uint32_t methodVA,std::int32_t sourceThisOffset) override {
        // A signed source subobject offset may designate a different
        // genuine owner. Never reinterpret it as a native pointer.
        if (sourceThisOffset!=0)
            return originals_.realOwner.call_original_direct_57c7ac(
                methodVA,sourceThisOffset);
        switch(methodVA) {
        case 0x57C27C:
            return originalManagerRouteChildUpdate_57c27c(
                controller_,a1_,originals_.routeChild);
        case 0x57C388:
            return originalManagerRoutedUpdate_57c388(
                controller_,a1_,originals_.routedChild);
        case 0x57C48C:
            return originalManagerFollowupUpdate_57c48c(
                controller_,a1_,originals_.followup);
        case 0x57C554:
            return originalManagerType2Update_57c554(
                controller_,a1_,originals_.type2);
        default:
            return originals_.realOwner.call_original_direct_57c7ac(
                methodVA,sourceThisOffset);
        }
    }
    std::uint32_t call_original_virtual_57c79c(
        std::uint32_t offset,std::int32_t sourceThisOffset) override {
        return originals_.realOwner.call_original_virtual_57c79c(
            offset,sourceThisOffset);
    }
    void call_original_30ffd0(void* parent,void* polygon) override {
        originals_.realOwner.call_original_30ffd0(parent,polygon);
    }
    std::uint8_t read_original_6354e7() override {
        return originals_.realOwner.read_original_6354e7();
    }
    std::uint8_t read_original_6354e0() override {
        return originals_.realOwner.read_original_6354e0();
    }
    void call_original_2478ec_247d84_warning() override {
        originals_.realOwner.call_original_2478ec_247d84_warning();
    }
private:
    OriginalManagerChild70& controller_;
    std::uint32_t a1_;
    OriginalManagerNativeMethodOwner& originals_;
};
}
std::uint32_t originalManagerDispatchNativeMethods_57c620(
    OriginalManagerChild70& controller,std::uint32_t originalA1,
    OriginalManagerNativeMethodOwner& owner) {
    ActualMethodCalls methodCalls(controller,originalA1,owner);
    return originalManagerDispatch_57c620(controller,originalA1,methodCalls);
}
} // namespace ae3
