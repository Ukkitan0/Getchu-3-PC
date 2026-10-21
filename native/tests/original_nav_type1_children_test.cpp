#include "ae3/original_nav_type1_children.hpp"
#include "ae3/original_nav_source_refs.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool yes) {if(!yes)throw std::runtime_error("Original 0x21B0B0/0x21B240 type-1 regression");}
static void put16(std::vector<std::byte>& b,size_t p,std::uint16_t n){
  b.at(p)=std::byte(n&255u);b.at(p+1)=std::byte(n>>8);
}
static std::uint32_t r32(const std::array<std::byte,0x70>& b,size_t p){
    return std::uint32_t(std::to_integer<std::uint8_t>(b.at(p)))|
      (std::uint32_t(std::to_integer<std::uint8_t>(b.at(p+1)))<<8)|
      (std::uint32_t(std::to_integer<std::uint8_t>(b.at(p+2)))<<16)|
      (std::uint32_t(std::to_integer<std::uint8_t>(b.at(p+3)))<<24);
}
int main(int argc,char**argv){
    // Known source layout: vector base at original source+0x40,
    // count/index lists at source+0x4C; entries are 16-byte SQ.
    std::vector<std::byte> bytes(0x300);
    const std::uint32_t vectorBase=0x80,lists=0x200;
    for(unsigned i=0;i<8;++i)
      for(unsigned j=0;j<16;++j)bytes[vectorBase+i*16+j]=std::byte((i*17+j)&255u);
    put16(bytes,lists+0,3);put16(bytes,lists+2,2);
    put16(bytes,lists+4,6);put16(bytes,lists+6,1);
    // A second selector points at the u16 word indexed by 'argument',
    // not the number of source bytes.
    put16(bytes,lists+8,0);
    OriginalStageSpatialLoaded graph{};
    graph.records.resize(2);
    graph.sourceHeader.relativeOffsets[2]=vectorBase;
    graph.sourceHeader.relativeOffsets[5]=lists;
    graph.pendingOriginalActions={{0,0,1,0},{0,1,2,0},{1,2,1,4}};
    const OriginalStageSpatialSource source{0x510000,bytes,0};
    ck(originalNavMaterializeType1_21d70c(source,graph)==2);
    ck(graph.originalType1Children.size()==2);
    ck(graph.pendingOriginalActions.size()==1&&graph.pendingOriginalActions[0].type==2);
    const auto& first=graph.originalType1Children[0];
    ck(first.sourceRecordIndex==0&&first.sourceActionIndex==0);
    ck(first.originalVectors.size()==3);
    for(unsigned k=0;k<3;++k){
      const auto id=std::array<unsigned,3>{2,6,1}[k];
      for(unsigned j=0;j<16;++j)
        ck(first.originalVectors[k][j]==bytes[vectorBase+id*16+j]);
    }
    ck(graph.originalType1Children[1].originalVectors.empty());
    ck(r32(graph.records[0].originalFields,4)==0);
    // The type-1 entry has not been assigned an invented guest VA.
    // The original type-2 binder remains independently functional.
    graph.sourceHeader.relativeOffsets[3]=0x180;
    ck(originalNavResolveType2_21d6dc(source,graph).size()==1);
    ck(graph.pendingOriginalActions.empty());
    ck(r32(graph.records[0].originalFields,0x14)==0x510180);
    ck(originalNavMaterializeType1_21d70c(source,graph)==0);
    ck(graph.originalType1Children.size()==2);
    // One malformed reference aborts the batch without modifying state.
    graph.pendingOriginalActions={{0,1,1,0},{1,1,1,0xfffe}};
    const auto before=graph.originalType1Children.size();
    bool thrown=false;
    try{(void)originalNavMaterializeType1_21d70c(source,graph);}
    catch(const std::out_of_range&){thrown=true;}
    ck(thrown&&graph.originalType1Children.size()==before&&graph.pendingOriginalActions.size()==2);
    graph.pendingOriginalActions={{2,1,1,0}};
    thrown=false;
    try{(void)originalNavMaterializeType1_21d70c(source,graph);}
    catch(const std::out_of_range&){thrown=true;}
    ck(thrown);
    graph.pendingOriginalActions={{0,3,1,0}};
    thrown=false;
    try{(void)originalNavMaterializeType1_21d70c(source,graph);}
    catch(const std::out_of_range&){thrown=true;}
    ck(thrown);
    // Source-retail asset DOES NOT contain type-1 actions. Verify that
    // property independently from the private woods_c PCK fixture.
    if(argc==2){
        std::ifstream file(argv[1],std::ios::binary);ck(bool(file));
        const std::vector<char> chars((std::istreambuf_iterator<char>(file)),{});
        ck(chars.size()==3328);
        auto read16=[&](size_t p){return std::uint16_t(std::uint8_t(chars.at(p)))|
            (std::uint16_t(std::uint8_t(chars.at(p+1)))<<8);};
        auto read32=[&](size_t p){return std::uint32_t(std::uint8_t(chars.at(p)))|
            (std::uint32_t(std::uint8_t(chars.at(p+1)))<<8)|
            (std::uint32_t(std::uint8_t(chars.at(p+2)))<<16)|
            (std::uint32_t(std::uint8_t(chars.at(p+3)))<<24);};
        unsigned type1=0,type2=0;
        for(unsigned fileIdx=0;fileIdx<2;++fileIdx){
          const size_t raw=(fileIdx?2224:96);
          const unsigned count=read32(raw+0x24);
          const size_t headers=raw+read32(raw+0x3c);
          for(unsigned n=0;n<count;++n){
             const size_t pos=headers+n*0x1e;
             const unsigned actions=read16(pos+2);ck(actions<=3);
             for(unsigned i=0;i<actions;++i){
               const auto kind=read16(pos+0x0a+i*2);
               type1+=(kind==1);type2+=(kind==2);
             }
          }
        }
        ck(type1==0&&type2==15);
    }
    std::mt19937 rng(0x21b0b0);
    for(int rep=0;rep<5000;++rep){
        const auto n=static_cast<std::uint16_t>(rng()%8);
        put16(bytes,lists,n);
        for(std::uint16_t i=0;i<n;++i)put16(bytes,lists+2+i*2,static_cast<std::uint16_t>(rng()%8));
        OriginalStageSpatialLoaded fresh{};
        fresh.records.resize(1);
        fresh.sourceHeader.relativeOffsets[2]=vectorBase;
        fresh.sourceHeader.relativeOffsets[5]=lists;
        fresh.pendingOriginalActions={{0,2,1,0}};
        ck(originalNavMaterializeType1_21d70c(source,fresh)==1);
        ck(fresh.originalType1Children[0].originalVectors.size()==n);
        for(std::uint16_t i=0;i<n;++i){
            const auto id=std::to_integer<unsigned>(bytes[lists+2+i*2])|
                (std::to_integer<unsigned>(bytes[lists+3+i*2])<<8);
            ck(fresh.originalType1Children[0].originalVectors[i][0]==bytes[vectorBase+id*16]);
        }
    }
}
