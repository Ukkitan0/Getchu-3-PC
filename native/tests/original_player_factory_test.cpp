#include "ae3/original_player_factory.hpp"
#include <stdexcept>
#include <string_view>
#include <vector>
using namespace ae3;
static void require(bool x){if(!x)throw std::runtime_error("Original Player resource routing differs from ELF");}
int main(){
    int objects[10]{};
    OriginalSecondaryFactoryGlobal active{&objects[0],0xa7},inactive{};
    OriginalPlayerResources p{&objects[0],&objects[1],&objects[2],&objects[3],
        &objects[4],&objects[5],&objects[6],&objects[7],&objects[8],&objects[9]};
    const std::vector<std::string_view> keys={"player","main_player","item_chatcher",
        "force","bar_interface","iZone","iHanger","iRope","iDashBall","iHeroSwitch"};
    for(unsigned i=0;i<keys.size();++i){
        const auto key=originalResourceName16(keys[i]);
        require(originalPlayerResourceLookup_34a014(p,key,active)==&objects[i]);
        require(originalPlayerResourceLookup_34a014(p,key,inactive)==(i<4?&objects[i]:nullptr));
    }
    require(originalPlayerResourceLookup_34a014(p,originalResourceName16("nonexistent"),active)==nullptr);
    auto modified=originalResourceName16("main_player");modified[15]=1;
    require(originalPlayerResourceLookup_34a014(p,modified,active)==nullptr);
    auto mixed=originalResourceName16("main_player");mixed[0]='M';
    require(originalPlayerResourceLookup_34a014(p,mixed,active)==nullptr);
    bool rejected=false;
    try{(void)originalResourceName16("0123456789abcdefghijklmnop");}
    catch(const std::invalid_argument&){rejected=true;}
    require(rejected);
    require(originalSecondaryFactoryGet_37d348(active)==&objects[0]);
    require(originalSecondaryFactoryByte_37d360(active)==0xa7);
    originalSecondaryFactoryReset_37d354(active);
    require(originalSecondaryFactoryGet_37d348(active)==nullptr);
    require(originalSecondaryFactoryByte_37d360(active)==0);
    OriginalPlayerResources unconstructed{};
    require(originalPlayerResourceLookup_34a014(unconstructed,originalResourceName16("player"),active)==nullptr);
}
