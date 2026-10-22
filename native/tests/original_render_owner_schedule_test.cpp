#include "ae3/original_render_owner_schedule.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace ae3;
namespace {
void check(bool a,const char* b){if(!a)throw std::runtime_error(b);}
void run(){
    OriginalRetailRenderOwner owner;
    owner.schedulingMode38=1;owner.schedulingPeriod3c=3;
    owner.slots={{10,1,1},{11,0,0},{12,1,1},{13,1,1},{14,1,0}};
    std::vector<OriginalRenderId> validated;
    auto init=[&](OriginalRetailRenderSlot& s){validated.push_back(s.id);s.active05=1;};
    auto a=advanceOriginalRetailRenderOwner(owner,init);
    check(a.frame==1&&a.validated==1&&a.active==4,"original owner init/frame count wrong");
    check(a.scheduledIds==std::vector<OriginalRenderId>{11},
          "64-bit frame modulo must select second active slot on frame 1");
    check(validated==std::vector<OriginalRenderId>{11},"only zero-flag slot receives source validator");
    check(std::all_of(owner.slots.begin(),owner.slots.end(),[](auto&s){return s.pendingValidation04==0;}),
          "retail record byte4 must reset even when byte5 is true");
    a=advanceOriginalRetailRenderOwner(owner,init);
    check(a.frame==2&&a.scheduledIds==std::vector<OriginalRenderId>{12},
          "original frame 2 modulo slot routing wrong");
    a=advanceOriginalRetailRenderOwner(owner,init);
    check(a.frame==3&&a.scheduledIds==std::vector<OriginalRenderId>{10},
          "original frame 3 modulo slot routing wrong");
    owner.schedulingMode38=0;owner.schedulingPeriod3c=-12;
    a=advanceOriginalRetailRenderOwner(owner,init);
    check(a.scheduledIds==std::vector<OriginalRenderId>({10,11,12,13,14}),
          "ordinary retail mode schedules all activated records");
    owner.schedulingMode38=1;
    a=advanceOriginalRetailRenderOwner(owner,init);
    check(a.scheduledIds==std::vector<OriginalRenderId>{10},
          "nonpositive retail period must be clamped to 1");
    owner.frame40=UINT64_MAX;
    a=advanceOriginalRetailRenderOwner(owner,init);
    check(a.frame==0&&a.scheduledIds==std::vector<OriginalRenderId>{10},
          "owner frame64 counter must preserve machine wraparound");
    owner.slots[0].pendingValidation04=0;
    bool failed=false;
    try{advanceOriginalRetailRenderOwner(owner,{});}catch(const std::runtime_error&){failed=true;}
    check(failed,"unavailable original slot initializer was silently skipped");
    owner.slots[0].pendingValidation04=1;owner.slots[1].id=10;
    failed=false;
    try{advanceOriginalRetailRenderOwner(owner,init);}catch(const std::runtime_error&){failed=true;}
    check(failed,"duplicated retail owner IDs not rejected");
}
}
int main(){
    try{run();std::cout<<"ORIGINAL_RENDER_OWNER_SCHEDULE_PASS: 64bit_counter interval modulo active_ordinals validation_byte reset\n";return 0;}
    catch(const std::exception& e){std::cerr<<"ORIGINAL_RENDER_OWNER_SCHEDULE_FAIL: "<<e.what()<<"\n";return 1;}
}
