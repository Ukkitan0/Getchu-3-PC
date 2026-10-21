#include "ae3/original_resource_broadcast.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool b){if(!b)throw std::runtime_error("Original resource broadcast test failed");}
struct PlainSink final:OriginalResourceEventSink<int>{
    std::vector<int> observed;
    void originalOnResource(const int& x) override {observed.push_back(x);}
};
struct ModifyingSink final:OriginalResourceEventSink<int>{
    OriginalResourceBucketNode<int>* head=nullptr;
    std::vector<int> values;
    explicit ModifyingSink(OriginalResourceBucketNode<int>* first):head(first){}
    void originalOnResource(const int& x) override {
        values.push_back(x);
        if(x==1)head->next=nullptr;
    }
};
struct RegistryEvents final:OriginalNavrangeRegistrationEvents {
    int installed=0,teardown=0,reset=0;
    void originalInstallRequest_27242c(std::uint32_t va) override {
        ck(va==OriginalNavrangeInstallTableVA);++installed;
    }
    void originalTeardownRequest_36d844(std::uint32_t va) override {
        ck(va==OriginalNavrangeTeardownTableVA);++teardown;
    }
    void originalResetDescriptorVtable_4e3d9c(std::uint32_t va) override {
        ck(va==OriginalNavrangeFinalCleanupTableVA);++reset;
    }
};
static void w32(std::span<std::byte> out,std::size_t at,std::uint32_t v){
    for(unsigned j=0;j<4;++j)out[at+j]=std::byte((v>>(j*8))&255u);
}
static void w16(std::span<std::byte> out,std::size_t at,std::uint16_t v){
    out[at]=std::byte(v&255u);out[at+1]=std::byte(v>>8);
}
int main(){
    static_assert(OriginalResourceActiveMethods[0]==0x518e4cu);
    static_assert(OriginalResourceActiveMethods[3]==0x518e24u);
    static_assert(OriginalResourceTransitionMethods[0]==0x518e4cu);
    static_assert(OriginalResourceBaseDescriptorVA==0x6ae290u);
    OriginalResourceBucketHeads<int> buckets{};
    PlainSink plain;
    ck(originalResourceDispatch_27242c(buckets,plain)==0);
    int a=1,b=2,c=3,d=4,e=5;
    OriginalResourceBucketNode<int> n0{&a,nullptr},n1{&b,nullptr},
        n2{&c,nullptr},n3{&d,nullptr},n4{&e,nullptr};
    n0.next=&n1;n1.next=&n2;
    buckets[0]=&n0;buckets[1]=&n3;buckets[127]=&n4;
    ck(originalResourceDispatch_27242c(buckets,plain)==5);
    ck(plain.observed==std::vector<int>({1,2,3,4,5}));
    ModifyingSink modifier(&n0);
    ck(originalResourceDispatch_27242c(buckets,modifier)==3);
    ck(modifier.values==std::vector<int>({1,4,5}));
    buckets[0]=nullptr;buckets[1]=nullptr;buckets[127]=nullptr;
    OriginalResourceBucketNode<int> invalid{nullptr,nullptr};buckets[4]=&invalid;
    bool rejected=false;
    try{(void)originalResourceDispatch_27242c(buckets,plain);}
    catch(const std::invalid_argument&){rejected=true;}
    ck(rejected);
    RegistryEvents events; OriginalNavrangeRegisteredOwner owner;
    owner.originalInitialize_21f8d0(events);
    std::array<std::byte,0x64> header{};
    constexpr char magic[]="navrange";
    for(unsigned i=0;i<8;i++)header[i]=std::byte(static_cast<unsigned char>(magic[i]));
    w32(header,0x3c,std::bit_cast<std::uint32_t>(1.f));
    w32(header,0x40,std::bit_cast<std::uint32_t>(2.f));
    w32(header,0x44,std::bit_cast<std::uint32_t>(3.f));
    w32(header,0x48,std::bit_cast<std::uint32_t>(4.f));
    w32(header,0x54,1);w32(header,0x5c,0x650000u);
    std::array<std::byte,0x20> key{};key[0]=std::byte{0x44};
    std::array<OriginalNavGroupAllocation,1> allocation{{{0x650000u,key}}};
    std::array<OriginalSpatialRecord70,3> records{};
    std::array<std::byte,0x110> nav{};
    w32(nav,0x24,3);w32(nav,0x3c,0x80);
    for(int i=0;i<3;i++){
        w16(nav,0x80+0x1e*i+0x16,1);
        w32(records[i].originalFields,0x40,
            std::bit_cast<std::uint32_t>(float(i==2?7:i)));
    }
    OriginalNavrangeResourceDelivery valid{{header,allocation},records,nav};
    auto otherHeader=header;otherHeader[0]=std::byte{'d'};
    OriginalNavrangeResourceDelivery other{{otherHeader,allocation},records,nav};
    OriginalResourceBucketNode<OriginalNavrangeResourceDelivery> v{&valid,nullptr};
    OriginalResourceBucketNode<OriginalNavrangeResourceDelivery> o{&other,&v};
    OriginalResourceBucketHeads<OriginalNavrangeResourceDelivery> groupBuckets{};
    groupBuckets[0]=&o;
    auto first=originalNavrangeDispatchRegistered_27242c(owner,groupBuckets);
    ck(first.resourcesVisited==2&&first.navrange.sourceGroups==1);
    ck(first.navrange.ownersCreated==1&&first.navrange.sourcePolygonsInserted==2);
    ck(owner.originalRegistry().size()==1);
    auto repeat=originalNavrangeDispatchRegistered_27242c(owner,groupBuckets);
    ck(repeat.resourcesVisited==2&&repeat.navrange.sourcePolygonsInserted==0);
    owner.originalShutdown_21f940(events);
    ck(events.installed==1&&events.teardown==1&&events.reset==1);
}
