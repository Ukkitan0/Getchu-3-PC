#include "ae3/original_exdb_package.hpp"
#include <bit>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool x, const char* msg){if(!x)throw std::runtime_error(msg);}
static std::vector<std::byte> load(const char* path){
    std::ifstream f(path,std::ios::binary);
    if(!f)throw std::runtime_error("Private retail exdb.pck missing");
    const std::vector<char> raw((std::istreambuf_iterator<char>(f)),{});
    std::vector<std::byte> data;data.reserve(raw.size());
    for(char v:raw)data.push_back(std::byte(static_cast<unsigned char>(v)));
    return data;
}
int main(int argc,char**argv){
    if(argc!=2)throw std::invalid_argument("Pass path to private woods_c/exdb.pck");
    const auto bytes=load(argv[1]);
    const auto items=originalExdbPackageEntriesFromPck(bytes);
    ck(items.size()==2,"Real woods_c PCK has two entries");
    ck(items[0].name=="monkeyprofile.exdb"&&items[0].type=="exdb", "First source asset name/type");
    ck(items[0].packageDataOffset==0x60&&items[0].originalBytes.size()==0x1400,"Monkey profile source offset");
    ck(items[1].name=="MainCameraParam.exdb"&&items[1].type=="exdb","Second source asset name/type");
    ck(items[1].packageDataOffset==0x1460&&items[1].originalBytes.size()==0x200,"Camera source offsets");
    ck(originalBgmExdbMatchingEntries_35fd20(items).empty(),"The woods_c PCK contains no bgm_desc.exdb");
    OriginalExdbDecodedDocument camera;
    camera.decode(items[1].originalBytes);
    const auto layout=camera.originalRecordIndex();
    ck(layout.sourceCountAt20==1 && layout.sourceStrideAt24==160 &&
       layout.sourceRecordBaseAt04==352,"Retail camera record index");
    ck(camera.decodedAttributeRoot()!=nullptr,"Retail camera property tree");
    const auto rec=camera.record(0);
    ck(rec.size()==160,"Retail camera record length");
    ck(originalAttributeString_2bb3bc(rec,camera.decodedAttributeRoot(),"name","invalid")=="camparam_default", "Retail camparam name");
    ck(originalAttributeString_2bb3bc(rec,camera.decodedAttributeRoot(),"area","invalid")=="woods_c", "Retail camera area");
    ck(originalAttributeFloat_2bb1d0(rec,camera.decodedAttributeRoot(),"lkdown_ang",-1.f)==30.f,"Retail camera angle");
    ck(originalAttributeFloat_2bb1d0(rec,camera.decodedAttributeRoot(),"distance",-2.f)==-2.f,"Retail missing sentinel fallback");
    ck(originalAttributeFloat_2bb1d0(rec,camera.decodedAttributeRoot(),"not_present",7.f)==7.f,"Absent field fallback");
    OriginalExdbDecodedDocument monkey;
    monkey.decode(items[0].originalBytes);
    const auto monkeyIndex=monkey.originalRecordIndex();
    ck(monkeyIndex.sourceCountAt20==4 && monkeyIndex.sourceStrideAt24==1024 &&
       monkeyIndex.sourceRecordBaseAt04==1024,"Retail monkey source header");
    ck(monkey.record(3).size()==1024,"Fourth original monkey profile exists");
    ck(originalAttributeString_2bb3bc(monkey.record(0),monkey.decodedAttributeRoot(),"name","missing")!="missing", "Retail monkey name");
    // Strict table bounds and malformed string offsets; no off-package reads.
    auto truncated=bytes;truncated.resize(70);
    bool caught=false;
    try{(void)originalExdbPackageEntriesFromPck(truncated);}catch(const std::out_of_range&){caught=true;}
    ck(caught,"PCK truncation rejected");
    auto malformed=bytes;malformed[0x40]=std::byte{0xff};
    caught=false;
    try{(void)originalExdbPackageEntriesFromPck(malformed);}catch(const std::out_of_range&){caught=true;}
    ck(caught,"Bad original name offset rejected");
}
