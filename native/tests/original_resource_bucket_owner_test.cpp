#include "ae3/original_resource_bucket_owner.hpp"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool b){if(!b)throw std::runtime_error("original resource bucket insertion mismatch");}
struct Sink final:OriginalResourceEventSink<int>{
    std::vector<int> values;
    void originalOnResource(const int& v) override {values.push_back(v);}
};
struct Events final:OriginalNavrangeRegistrationEvents {
    void originalInstallRequest_27242c(std::uint32_t) override {}
    void originalTeardownRequest_36d844(std::uint32_t) override {}
    void originalResetDescriptorVtable_4e3d9c(std::uint32_t) override {}
};
static OriginalResourceKey32 key(unsigned value) {
    OriginalResourceKey32 result{};
    result[0]=std::byte(value&255u);
    return result;
}
int main(){
    static_assert(sizeof(OriginalResourceKey32)==32);
    OriginalResourceBucketOwner<int> owner;
    Sink sink;
    ck(owner.size()==0&&owner.originalBroadcast_36d844(sink)==0);
    ck(originalResourceBucketIndex_36cfd0_36d6a0(key(127))==127);
    ck(originalResourceBucketIndex_36cfd0_36d6a0(key(128))==0);
    ck(originalResourceBucketIndex_36cfd0_36d6a0(key(255))==127);
    OriginalResourceKey32 everyByte{};
    everyByte.fill(std::byte{0xff});
    ck(originalResourceBucketIndex_36cfd0_36d6a0(everyByte)==96);
    for(unsigned i=0;i<32;++i)everyByte[i]=std::byte(i);
    ck(originalResourceBucketIndex_36cfd0_36d6a0(everyByte)==112);
    const int a=12,b=13,c=14,d=15,e=16;
    const auto& first=owner.originalInsert_36d610(key(127),a);
    const auto& second=owner.originalInsert_36d610(key(255),b);
    owner.originalInsert_36d610(key(0),c);
    owner.originalInsert_36d610(key(1),d);
    const auto& duplicate=owner.originalInsert_36d610(key(127),e);
    ck(first.originalBucket==127&&second.originalBucket==127);
    ck(owner.size()==5);
    const auto& heads=owner.originalBuckets();
    ck(heads[0]&&heads[0]->resource==&c);
    ck(heads[1]&&heads[1]->resource==&d);
    ck(heads[127]&&heads[127]->resource==&a);
    ck(heads[127]->next&&heads[127]->next->resource==&b);
    ck(heads[127]->next->next&&heads[127]->next->next->resource==&e);
    ck(heads[127]->next->next->next==nullptr);
    ck(owner.originalBroadcast_36d844(sink)==5);
    ck(sink.values==std::vector<int>({14,15,12,13,16}));
    ck(first.originalKey!=second.originalKey);
    ck(first.originalKey==duplicate.originalKey);
    // Real navrange receiver consumes the very same original buckets.
    OriginalResourceBucketOwner<OriginalNavrangeResourceDelivery> navOwner;
    OriginalNavrangeRegisteredOwner navRegistry;
    Events events;
    navRegistry.originalInitialize_21f8d0(events);
    OriginalNavrangeResourceDelivery dummy{};
    navOwner.originalInsert_36d610(key(12),dummy);
    navOwner.originalInsert_36d610(key(12),dummy);
    const auto navResult=originalNavrangeDispatchRegistered_27242c(
        navRegistry,navOwner.originalBuckets());
    ck(navResult.resourcesVisited==2);
    navRegistry.originalShutdown_21f940(events);
    navOwner.originalClear();
    owner.originalClear();
    ck(owner.size()==0&&owner.originalBuckets()[127]==nullptr);
    sink.values.clear();
    ck(owner.originalBroadcast_36d844(sink)==0);
    // Source-byte formula checked on 10,000 deterministic keys.
    std::uint32_t state=0x96edc3abu;
    for(unsigned n=0;n<10000;++n){
        OriginalResourceKey32 randomKey{};
        unsigned expected=0;
        for(auto& byte:randomKey){
            state=state*1664525u+1013904223u;
            const auto value=static_cast<std::uint8_t>(state>>24);
            byte=std::byte{value};expected+=value;
        }
        ck(originalResourceBucketIndex_36cfd0_36d6a0(randomKey)==(expected&127u));
    }
}
