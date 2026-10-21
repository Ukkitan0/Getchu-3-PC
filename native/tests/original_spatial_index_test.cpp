#include "ae3/original_spatial_index.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void check(bool b){if(!b)throw std::runtime_error("Original spatial source translation regression");}
struct SourceCalls final:OriginalSpatialEngineCalls {
    const OriginalSpatialRecord70* base=nullptr;
    std::vector<bool> enabled;
    std::vector<float> distance;
    std::vector<std::size_t> gates,scored;
    std::size_t index(const OriginalSpatialRecord70& x) const {
        return static_cast<std::size_t>(&x-base);
    }
    bool call_original_21b788_eligibility(const OriginalSpatialRecord70& x) override {
        auto i=index(x);gates.push_back(i);return enabled.at(i);
    }
    float call_original_21b468_and_VU_distance(const OriginalSpatialRecord70& x) override {
        auto i=index(x);scored.push_back(i);return distance.at(i);
    }
};
struct Destruction final:OriginalSpatialDestroyCalls {
    OriginalSpatialRecord70* base=nullptr;
    std::vector<std::string> calls;
    void call_original_41a3b0_free(void* p) override {
        check(p==reinterpret_cast<void*>(0x10));calls.push_back("free0");
    }
    void call_original_21b384_record_destructor(OriginalSpatialRecord70& p) override {
        calls.push_back("dtor"+std::to_string(&p-base));
    }
    void call_original_36ede8_release(void* p) override {
        if(p==reinterpret_cast<void*>(0x20))calls.push_back("freerecords");
        else if(p==reinterpret_cast<void*>(0x30))calls.push_back("freeaux");
        else throw std::runtime_error("Unexpected allocator pointer");
    }
};
int main(){
    std::vector<OriginalSpatialRecord70> records(6);
    std::uint16_t hitIds[]={4,1,3,5,2,0};
    SourceCalls calls{};
    calls.base=records.data();
    calls.enabled={false,true,true,false,true,true};
    calls.distance={3.f,4.f,2.f,100.f,6.f,1.f};
    check(originalSpatialNearest_21d898(records,OriginalSpatialGridHits{hitIds},calls)==&records[5]);
    check(calls.gates==std::vector<std::size_t>({4,1,3,5,2,0}));
    check(calls.scored==std::vector<std::size_t>({4,1,5,2}));
    calls.gates.clear();calls.scored.clear();
    check(originalSpatialNearest_21d898(records,OriginalSpatialGridHits{std::span<const std::uint16_t>{}},calls)==nullptr);
    check(calls.gates.empty()&&calls.scored.empty());
    calls.enabled.assign(6,false);
    check(originalSpatialNearest_21d898(records,OriginalSpatialGridHits{hitIds},calls)==nullptr);
    check(calls.scored.empty());
    calls.enabled.assign(6,true);calls.distance.assign(6,5.f);
    check(originalSpatialNearest_21d898(records,OriginalSpatialGridHits{hitIds},calls)==&records[4]);
    calls.distance[4]=std::numeric_limits<float>::quiet_NaN();
    check(originalSpatialNearest_21d898(records,OriginalSpatialGridHits{hitIds},calls)==&records[4]);
    calls.distance={3.f,4.f,2.f,100.f,6.f,1.f};calls.gates.clear();calls.scored.clear();
    check(originalSpatialNearestAll_21da2c(records,calls)==&records[5]);
    check(calls.gates.empty()&&calls.scored==std::vector<std::size_t>({0,1,2,3,4,5}));
    calls.scored.clear();calls.distance.assign(6,7.f);
    check(originalSpatialNearestAll_21da2c(records,calls)==&records[0]);
    calls.distance[0]=std::numeric_limits<float>::quiet_NaN();
    check(originalSpatialNearestAll_21da2c(records,calls)==&records[0]);
    calls.scored.clear();
    check(originalSpatialNearestAll_21da2c(std::span<const OriginalSpatialRecord70>{},calls)==nullptr);
    check(calls.scored.empty());
    check(originalSpatialRecord_21d884(records,3)==&records[3]);
    check(originalSpatialByEntry_21b404(records,std::span<const std::uint16_t>(hitIds),2)==&records[3]);
    check(originalSpatialByEntry_21b404(records,std::span<const std::uint16_t>(hitIds),6)==nullptr);
    bool invalid=false;
    try{(void)originalSpatialRecord_21d884(records,65535);}
    catch(const std::out_of_range&){invalid=true;}
    check(invalid);
    for(std::size_t i=0;i<0x70;i++)records[3].originalFields[i]=std::byte(i);
    for(std::size_t j=0;j<3;j++){
        const auto v=originalSpatialVector_21b458(records[3],j);
        for(std::size_t k=0;k<16;k++)check(v[k]==std::byte(0x40+j*16+k));
    }
    std::uint16_t headerBytes[]={99,2,77};
    check(originalSpatialHeaderHalfword_21b444(headerBytes,1)==2);
    OriginalSpatialWordData data{};
    for(std::size_t i=0;i<8;i++)data.originalWords[i]=std::uint32_t(i*100+7);
    check(originalSpatialWordAt04_21b4e0(data,3)==407);
    check(originalSpatialWordAt10_21b4f0(data,2)==607);
    check(originalSpatialWordAt1C_21b500(data)==707);
    Destruction destroy{};destroy.base=records.data();
    OriginalSpatialRegistryOwner state{reinterpret_cast<void*>(0x10),reinterpret_cast<void*>(0x20),
        5,reinterpret_cast<void*>(0x30),3,std::span<OriginalSpatialRecord70>(records)};
    originalSpatialRegistryDestroy_21d7d8(state,destroy);
    check(destroy.calls==std::vector<std::string>({"free0","dtor0","dtor1","dtor2","freerecords","freeaux"}));
    check(!state.ownedBlock00&&!state.originalRecordAllocation04&&!state.auxiliaryAllocation10);
    check(state.selectionIndex0c==-1 && state.recordCount14==3);
    destroy.calls.clear();originalSpatialRegistryDestroy_21d7d8(state,destroy);
    check(destroy.calls.empty());
    state.ownedBlock00=reinterpret_cast<void*>(0x10);state.selectionIndex0c=7;
    originalSpatialRegistryDestroy_21d7d8(state,destroy);
    check(state.selectionIndex0c==7&&destroy.calls==std::vector<std::string>({"free0"}));
    destroy.calls.clear();state.originalRecordAllocation04=reinterpret_cast<void*>(0x20);
    state.recordCount14=0;
    originalSpatialRegistryDestroy_21d7d8(state,destroy);
    check(destroy.calls==std::vector<std::string>({"freerecords"}));
}
