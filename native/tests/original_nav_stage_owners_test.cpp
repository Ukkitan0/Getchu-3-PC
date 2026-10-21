#include "ae3/original_nav_stage_owners.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool ok){if(!ok)throw std::runtime_error("Original retail nav owner regression");}
int main(int argc,char**argv){
    ck(argc==2);
    std::ifstream file(argv[1],std::ios::binary);ck(bool(file));
    std::vector<char> raw((std::istreambuf_iterator<char>(file)),{});
    std::vector<std::byte> bytes;
    for(unsigned char c:raw)bytes.push_back(std::byte{c});
    auto sources=originalNavReadNativePck_4f0a5c(bytes);
    ck(sources.size()==2 && sources[0].originalSlot==0 && sources[1].originalSlot==1);
    OriginalNavStageOwners_21ed30 stage;
    ck(!stage.initialized());
    bool fail=false;
    try{(void)stage.originalAt(0);}catch(const std::out_of_range&){fail=true;}
    ck(fail);
    stage.originalInitialize_21ed30();
    ck(stage.initialized());
    for(int i=0;i<3;++i){
        const auto& slot=stage.originalAt(i);
        ck(!slot.active() && slot.sourceFields.sourceGridIndexAt0c==-1);
        ck(!slot.sourceFields.sourceGridPointerPresent);
    }
    fail=false;
    try{stage.originalAttach_4f0a5c(sources[0],{"nav_monkey.nav",0,0});}
    catch(const std::invalid_argument&){fail=true;}
    ck(fail && !stage.originalAt(0).active());
    const OriginalNavRuntimeSourceBinding_4f0a5c a{"nav_monkey.nav",0x510000,0x620000};
    const OriginalNavRuntimeSourceBinding_4f0a5c b{"nav_enemy.nav",0x520000,0x620028};
    stage.originalAttach_4f0a5c(sources[0],a);
    stage.originalAttach_4f0a5c(sources[1],b);
    ck(stage.originalAt(0).active() && stage.originalAt(1).active() && !stage.originalAt(2).active());
    ck(stage.originalAt(0).decodedRecordCount==39&&stage.originalAt(1).decodedRecordCount==20);
    ck(!stage.originalAt(0).sourceFields.sourceGridPointerPresent);
    ck(stage.originalAt(0).sourceFields.sourceRecordListPointerPresent);
    ck(stage.originalAt(0).sourceFields.sourceRecordCountAt14==39);
    ck(stage.originalAt(1).sourceFields.sourceRecordCountAt20==20);
    ck(stage.originalAt(0).originalSourceAllocation.size()==2128);
    bytes.clear();bytes.shrink_to_fit();sources.clear();sources.shrink_to_fit();
    ck(stage.originalAt(1).originalSourceAllocation.size()==1104);
    stage.originalReleaseSlot_21d7d8(0);
    ck(!stage.originalAt(0).active() && stage.originalAt(1).active());
    ck(stage.originalAt(0).sourceFields.sourceGridIndexAt0c==-1);
    ck(stage.originalAt(0).originalSourceAllocation.empty());
    stage.originalTeardown_21ed94();ck(!stage.initialized());
    stage.originalInitialize_21ed30();
    ck(stage.initialized() && !stage.originalAt(2).active());
    stage.originalTeardown_21ed94();stage.originalTeardown_21ed94();
    ck(!stage.initialized());
}
