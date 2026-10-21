#include "ae3/original_resource_record_batch.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>
using namespace ae3;
struct Sink final:OriginalResourceEventSink<OriginalResourceRecordB0>{
    std::vector<const std::byte*> raw;
    std::vector<std::size_t> indices;
    void originalOnResource(const OriginalResourceRecordB0& item) override {
        assert(item.originalBytes.size()==0xb0);
        raw.push_back(item.originalBytes.data());
        indices.push_back(item.sourceIndex);
    }
};
int main() {
    static_assert(OriginalResourceRecordStride_1bb4e4==176);
    OriginalResourceRecordBatchOwner owner;
    Sink sink;
    assert(owner.originalAppendPopulatedRecords_1bb3ec({},0)==0);
    assert(owner.originalAppendPopulatedRecords_1bb3ec({},-1)==0);
    assert(owner.originalBroadcast_36d844(sink)==0);
    std::array<std::byte,0xb0*3> pool{};
    pool[0]=std::byte{3};
    pool[0xb0]=std::byte{1};
    pool[0xb0+1]=std::byte{2};
    pool[2*0xb0]=std::byte{127};
    pool[0x20]=std::byte{0x51};
    pool[0xb0+0x21]=std::byte{0x52};
    assert(owner.originalAppendPopulatedRecords_1bb3ec(pool,3)==3);
    assert(owner.size()==3);
    const auto& buckets=owner.originalBuckets();
    assert(buckets[3]!=nullptr);
    assert(buckets[3]->resource->originalBytes.data()==pool.data());
    assert(buckets[3]->next->resource->originalBytes.data()==pool.data()+0xb0);
    assert(buckets[3]->next->next==nullptr);
    assert(buckets[127]!=nullptr);
    assert(owner.originalBroadcast_36d844(sink)==3);
    assert((sink.indices==std::vector<std::size_t>{0,1,2}));
    assert((sink.raw==std::vector<const std::byte*>{pool.data(),pool.data()+0xb0,pool.data()+2*0xb0}));
    bool caught=false;
    try {
        (void)owner.originalAppendPopulatedRecords_1bb3ec(
            std::span<const std::byte>(pool).first(2*0xb0-1),2);
    }catch(const std::out_of_range&){caught=true;}
    assert(caught&&owner.size()==3);
    // A second retail batch appends rather than deduplicating.
    assert(owner.originalAppendPopulatedRecords_1bb3ec(pool,3)==3);
    assert(owner.size()==6);
    sink.raw.clear();sink.indices.clear();
    assert(owner.originalBroadcast_36d844(sink)==6);
    assert((sink.indices==std::vector<std::size_t>{0,1,0,1,2,2}));
    owner.originalClear();
    assert(owner.size()==0&&owner.originalBroadcast_36d844(sink)==0);
    // Verify references remain stable across many new node allocations.
    std::vector<std::byte> large(0xb0*4096);
    for(std::size_t i=0;i<4096;++i)
        large[i*0xb0]=std::byte(static_cast<unsigned char>(i%128));
    assert(owner.originalAppendPopulatedRecords_1bb3ec(large,4096)==4096);
    assert(owner.size()==4096);
    sink.raw.clear();sink.indices.clear();
    assert(owner.originalBroadcast_36d844(sink)==4096);
    assert(sink.raw.front()==large.data());
    assert(sink.raw.back()==large.data()+4095*0xb0);
}
