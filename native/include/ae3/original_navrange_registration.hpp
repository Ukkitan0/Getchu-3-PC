#pragma once
#include "ae3/original_navrange_recovered_eligibility.hpp"
#include <array>
#include <cstdint>
#include <stdexcept>

namespace ae3 {
// Original retail EE function pointers, expressed as DATA identifiers,
// never callable host pointers. Source table 0x6AE248 (24 bytes).
inline constexpr std::uint32_t OriginalNavrangeInstallTableVA = 0x6ae248u;
inline constexpr std::array<std::uint32_t,6> OriginalNavrangeInstallTable{{
    0x4e4060u, // source 'navrange' validator / group registration
    0x4e4050u, // vtable-pointer initializer, tail return
    0x4e4030u, // vtable-pointer initializer and owner release
    0u,0u,0u
}};
// Alternate original table at 0x6AE260 supplied during teardown.
inline constexpr std::uint32_t OriginalNavrangeTeardownTableVA=0x6ae260u;
inline constexpr std::array<std::uint32_t,6> OriginalNavrangeTeardownTable{{
    0x4e41bcu,0x4e3fdcu,0x4e3fbcu,0x4e3fecu,0u,0u
}};
inline constexpr std::uint32_t OriginalNavrangeFinalCleanupTableVA=0x6ae278u;
inline constexpr std::array<std::uint32_t,6> OriginalNavrangeFinalCleanupTable{{
    0x4e41bcu,0x4e3facu,0x4e3f8cu,0x41b8e8u,0u,0u
}};
inline constexpr std::uint32_t OriginalNavrangeOwnerGlobalVA=0x647b90u;

// Hooks only for original subsystems not yet recovered: the generic
// dispatcher installer 0x27242C and teardown request 0x36D844.
// A concrete stage loader must supply these; no assumed global singleton.
class OriginalNavrangeRegistrationEvents {
public:
    virtual ~OriginalNavrangeRegistrationEvents()=default;
    virtual void originalInstallRequest_27242c(
        std::uint32_t originalDescriptorVA)=0;
    virtual void originalTeardownRequest_36d844(
        std::uint32_t originalDescriptorVA)=0;
    virtual void originalFinalCleanup_4e3d9c(
        std::uint32_t originalDescriptorVA)=0;
};

// Native source-level ownership corresponding to original 0x21F8D0
// initialization and 0x21F940 teardown, retaining the real recovered
// registry and polygon eligibility code. It does not emulate EE memory.
class OriginalNavrangeRegisteredOwner final {
public:
    void originalInitialize_21f8d0(OriginalNavrangeRegistrationEvents& events) {
        if(active_)throw std::logic_error("navrange owner already initialized");
        // Construct original map owner BEFORE dispatch registration.
        active_=true;
        try {events.originalInstallRequest_27242c(OriginalNavrangeInstallTableVA);}
        catch(...){active_=false;throw;}
    }
    OriginalNavGroupBuildResult originalDispatchResource_4e4060(
        const OriginalNavGroupSource& source,
        std::span<const OriginalSpatialRecord70> polygons,
        std::span<const std::byte> originalNav) {
        if(!active_)throw std::logic_error("navrange resource before registry startup");
        return originalNavBuildGroupsRecovered_4e4060(owner_,source,polygons,originalNav);
    }
    void originalShutdown_21f940(OriginalNavrangeRegistrationEvents& events) {
        if(!active_)throw std::logic_error("navrange owner was not initialized");
        // Original 0x21F964 requests dispatcher change first. Only
        // afterwards 0x21F978/0x21F980 release the registry allocation.
        events.originalTeardownRequest_36d844(OriginalNavrangeTeardownTableVA);
        owner_.originalClear_4e3fec();
        active_=false;
        // 0x21F998 executes after the original global was cleared.
        events.originalFinalCleanup_4e3d9c(OriginalNavrangeFinalCleanupTableVA);
    }
    bool active() const noexcept {return active_;}
    OriginalNavGroupRegistry& originalRegistry() noexcept {return owner_;}
    const OriginalNavGroupRegistry& originalRegistry() const noexcept {return owner_;}
private:
    bool active_=false;
    OriginalNavGroupRegistry owner_{};
};
} // namespace ae3
