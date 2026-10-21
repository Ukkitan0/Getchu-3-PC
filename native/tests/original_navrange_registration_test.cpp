#include "ae3/original_navrange_registration.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
using namespace ae3;
static void ck(bool ok) {if(!ok)throw std::runtime_error("navrange retail dispatcher regression");}
static void w32(std::span<std::byte> b,std::size_t offset,std::uint32_t v) {
    for(unsigned i=0;i<4;++i)b[offset+i]=std::byte((v>>(i*8))&0xffu);
}
static void w16(std::span<std::byte> b,std::size_t off,std::uint16_t v) {
    b[off]=std::byte(v&255u); b[off+1]=std::byte(v>>8);
}
class Events final: public OriginalNavrangeRegistrationEvents {
public:
    unsigned installs=0,teardowns=0,finalCleanups=0;
    bool failInstall=false;
    void originalInstallRequest_27242c(std::uint32_t va) override {
        ck(va==OriginalNavrangeInstallTableVA);++installs;
        if(failInstall) throw std::runtime_error("controlled registration failure");
    }
    void originalTeardownRequest_36d844(std::uint32_t va) override {
        ck(va==OriginalNavrangeTeardownTableVA);++teardowns;
    }
    void originalResetDescriptorVtable_4e3d9c(std::uint32_t va) override {
        ck(va==OriginalNavrangeFinalCleanupTableVA);++finalCleanups;
    }
};
int main() {
    static_assert(OriginalNavrangeInstallTableVA==0x6ae248u);
    static_assert(OriginalNavrangeInstallTable[0]==0x4e4060u);
    static_assert(OriginalNavrangeTeardownTable[3]==0x4e3fecu);
    static_assert(OriginalNavrangeFinalCleanupTable[3]==0x41b8e8u);
    OriginalNavrangeRegisteredOwner reg;
    Events events;
    ck(!reg.active());
    bool failed=false;
    try{reg.originalShutdown_21f940(events);}catch(const std::logic_error&){failed=true;}
    ck(failed);
    events.failInstall=true;failed=false;
    try{reg.originalInitialize_21f8d0(events);}catch(const std::runtime_error&){failed=true;}
    ck(failed&&!reg.active()&&reg.originalRegistry().size()==0);
    events.failInstall=false;
    reg.originalInitialize_21f8d0(events);
    ck(reg.active()&&events.installs==2);
    failed=false;
    try{reg.originalInitialize_21f8d0(events);}catch(const std::logic_error&){failed=true;}
    ck(failed);
    std::array<std::byte,0x64> header{};
    constexpr char magic[]="navrange";
    for(unsigned i=0;i<8;++i)header[i]=std::byte(static_cast<unsigned char>(magic[i]));
    // Identity quaternion, positive extents, 1 source group.
    w32(header,0x3c,std::bit_cast<std::uint32_t>(1.f));
    w32(header,0x40,std::bit_cast<std::uint32_t>(2.f));
    w32(header,0x44,std::bit_cast<std::uint32_t>(3.f));
    w32(header,0x48,std::bit_cast<std::uint32_t>(4.f));
    w32(header,0x54,1);w32(header,0x5c,0x650000u);
    std::array<std::byte,0x20> keys{};keys[0]=std::byte{0x71};
    std::array<OriginalNavGroupAllocation,1> alloc{{{0x650000u,keys}}};
    OriginalNavGroupSource source{header,alloc};
    std::array<OriginalSpatialRecord70,3> records{};
    std::array<std::byte,0x110> nav{};
    w32(nav,0x24,3);w32(nav,0x3c,0x80);
    for(unsigned i=0;i<3;++i){
        w16(nav,0x80+0x1e*i+0x16,1);
        w32(records[i].originalFields,0x40,std::bit_cast<std::uint32_t>(float(i==2?7:i)));
    }
    const auto first=reg.originalDispatchResource_4e4060(source,records,nav);
    ck(first.sourceGroups==1&&first.ownersCreated==1&&first.sourcePolygonsInserted==2);
    ck(reg.originalRegistry().size()==1);
    const auto repeat=reg.originalDispatchResource_4e4060(source,records,nav);
    ck(repeat.ownersCreated==0&&repeat.sourcePolygonsInserted==0);
    reg.originalShutdown_21f940(events);
    ck(!reg.active()&&events.teardowns==1&&events.finalCleanups==1&&reg.originalRegistry().size()==0);
    failed=false;
    try{reg.originalDispatchResource_4e4060(source,records,nav);}catch(const std::logic_error&){failed=true;}
    ck(failed);
    reg.originalInitialize_21f8d0(events);
    ck(reg.active());
    const auto again=reg.originalDispatchResource_4e4060(source,records,nav);
    ck(again.ownersCreated==1&&again.sourcePolygonsInserted==2);
    reg.originalShutdown_21f940(events);
    ck(events.installs==3&&events.teardowns==2&&events.finalCleanups==2);
}
