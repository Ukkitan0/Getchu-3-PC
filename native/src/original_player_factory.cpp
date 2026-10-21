#include "ae3/original_player_factory.hpp"
#include <stdexcept>

namespace ae3 {
OriginalResourceName16 originalResourceName16(std::string_view name) {
    if(name.size()>16)throw std::invalid_argument("Original Player lookup key exceeds 16 bytes");
    OriginalResourceName16 result{};
    for(std::size_t i=0;i<name.size();++i)
        result[i]=static_cast<std::uint8_t>(name[i]);
    return result;
}
void* originalSecondaryFactoryGet_37d348(const OriginalSecondaryFactoryGlobal& g) noexcept {
    return g.instanceAt_6a99bc;
}
void originalSecondaryFactoryReset_37d354(OriginalSecondaryFactoryGlobal& g) noexcept {
    g.instanceAt_6a99bc=nullptr;
}
std::uint8_t originalSecondaryFactoryByte_37d360(const OriginalSecondaryFactoryGlobal& g) noexcept {
    return g.instanceAt_6a99bc?g.instanceByte8a0:0;
}
void* originalPlayerResourceLookup_34a014(
    const OriginalPlayerResources& p,const OriginalResourceName16& key,
    const OriginalSecondaryFactoryGlobal& global) noexcept {
    // Actual Player main virtual[2] at 0x34A014, exact 16-byte equality
    // and return paths. Original names located at 0x7121E0..0x712270.
    if(key==originalResourceName16("player"))return p.player_at_5c;
    if(key==originalResourceName16("main_player"))return p.main_player_at_60;
    if(key==originalResourceName16("item_chatcher"))return p.item_chatcher_embedded_64;
    if(key==originalResourceName16("force"))return p.force_embedded_180;
    // Original 0x34A0CC calls 0x37D348; zero return stops lookup here.
    if(!originalSecondaryFactoryGet_37d348(global))return nullptr;
    if(key==originalResourceName16("bar_interface"))return p.bar_interface_embedded_70;
    if(key==originalResourceName16("iZone"))return p.iZone_embedded_110;
    if(key==originalResourceName16("iHanger"))return p.iHanger_embedded_188;
    if(key==originalResourceName16("iRope"))return p.iRope_embedded_1a0;
    if(key==originalResourceName16("iDashBall"))return p.iDashBall_embedded_1d0;
    if(key==originalResourceName16("iHeroSwitch"))return p.iHeroSwitch_embedded_258;
    return nullptr;
}
} // namespace ae3
